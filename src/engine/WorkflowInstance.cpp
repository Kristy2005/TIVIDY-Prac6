#include "engine/WorkflowInstance.h"
#include <algorithm>
#include <sstream>

namespace tividy {

namespace {
bool moreUrgent(const WorkflowComponent* a, const WorkflowComponent* b) {
    return static_cast<int>(a->priority()) > static_cast<int>(b->priority());
}
}

WorkflowInstance::WorkflowInstance(const std::string& id, std::shared_ptr<const WorkflowDefinition> definition)
    : id_(id), definition_(definition), state_(&SubmittedState::instance()), escalationCount_(0),
      historyObserver_(history_), progress_(*this) {
    if (!definition_) throw WorkflowError("An instance needs a definition");
    tree_.reset(definition_->createWorkTree());
    attach(&historyObserver_);
    attach(&progress_);
    recordEvent(EventType::InstanceCreated, "", "system", "", "Created from definition '" + definition_->name() + "'");
}

WorkflowInstance::~WorkflowInstance() {}

// ---------------------------------------------------------------- participants / data

Participant& WorkflowInstance::addParticipant(const std::string& name, const std::string& role) {
    if (participants_.count(name)) throw WorkflowError("Participant '" + name + "' already exists");
    if (!definition_->hasRole(role)) throw WorkflowError("Role '" + role + "' is not part of '" + definition_->name() + "'");
    return participants_.insert(std::make_pair(name, Participant(name, Role(role)))).first->second;
}

Participant* WorkflowInstance::findParticipant(const std::string& name) {
    std::map<std::string, Participant>::iterator it = participants_.find(name);
    return it == participants_.end() ? 0 : &it->second;
}

std::vector<Participant*> WorkflowInstance::participants() {
    std::vector<Participant*> out;
    for (std::map<std::string, Participant>::iterator it = participants_.begin(); it != participants_.end(); ++it)
        out.push_back(&it->second);
    return out;
}

void WorkflowInstance::setData(const std::string& key, const std::string& value) { data_[key] = value; }

std::string WorkflowInstance::getData(const std::string& key, const std::string& fallback) const {
    std::map<std::string, std::string>::const_iterator it = data_.find(key);
    return it == data_.end() ? fallback : it->second;
}

// ---------------------------------------------------------------- public actions -> state

void WorkflowInstance::start(const std::string& actor) { state_->start(*this, actor); }
void WorkflowInstance::assign(const std::string& itemId, const std::string& participant, const std::string& actor) {
    state_->assign(*this, itemId, participant, actor);
}
void WorkflowInstance::complete(const std::string& itemId, const std::string& actor, const std::string& outcome) {
    state_->complete(*this, itemId, actor, outcome);
}
void WorkflowInstance::cancelItem(const std::string& itemId, const std::string& actor, const std::string& reason) {
    state_->cancelItem(*this, itemId, actor, reason);
}
void WorkflowInstance::escalate(const std::string& actor, const std::string& reason) {
    state_->escalate(*this, actor, reason);
}
void WorkflowInstance::resolveEscalation(const std::string& actor, const std::string& notes) {
    state_->resolveEscalation(*this, actor, notes);
}
void WorkflowInstance::close(const std::string& actor, const std::string& reason) {
    state_->close(*this, actor, reason);
}

// ---------------------------------------------------------------- inspection

std::vector<WorkflowComponent*> WorkflowInstance::items() {
    std::vector<WorkflowComponent*> leaves;
    tree_->collectLeaves(leaves);
    return leaves;
}

WorkflowComponent* WorkflowInstance::findItem(const std::string& itemId) {
    WorkflowComponent* c = tree_->find(itemId);
    return (c && c->isLeaf()) ? c : 0;
}

std::vector<WorkflowComponent*> WorkflowInstance::availableWork() {
    std::vector<WorkflowComponent*> out, all = items();
    for (size_t i = 0; i < all.size(); ++i)
        if (all[i]->status() == WorkStatus::Available || all[i]->status() == WorkStatus::Assigned) out.push_back(all[i]);
    std::stable_sort(out.begin(), out.end(), moreUrgent);
    return out;
}

int WorkflowInstance::totalItems() const {
    std::vector<WorkflowComponent*> leaves;
    tree_->collectLeaves(leaves);
    return static_cast<int>(leaves.size());
}

int WorkflowInstance::finishedItems() const {
    std::vector<WorkflowComponent*> leaves;
    tree_->collectLeaves(leaves);
    int n = 0;
    for (size_t i = 0; i < leaves.size(); ++i)
        if (isTerminal(leaves[i]->status())) ++n;
    return n;
}

bool WorkflowInstance::allWorkFinished() const { return finishedItems() == totalItems(); }

std::string WorkflowInstance::describe() {
    std::ostringstream os;
    os << "Instance " << id_ << " (" << definition_->name() << ")  state: " << state_->name()
       << "  progress: " << progress_.summary() << "\n";
    if (!escalatedTo_.empty()) os << "  escalated to: " << escalatedTo_ << " (" << escalationReason_ << ")\n";
    std::vector<WorkflowComponent*> all = items();
    for (size_t i = 0; i < all.size(); ++i) {
        os << "  - " << all[i]->id() << "  " << all[i]->displayName() << "  [" << all[i]->status() << "]";
        if (!all[i]->assignee().empty()) os << " -> " << all[i]->assignee();
        if (!all[i]->outcome().empty()) os << " (" << all[i]->outcome() << ")";
        os << "\n";
    }
    return os.str();
}

// ---------------------------------------------------------------- memento

WorkflowMemento WorkflowInstance::createMemento() {
    WorkflowMemento m;
    m.stateName_ = state_->name();
    std::vector<WorkflowComponent*> all = items();
    for (size_t i = 0; i < all.size(); ++i) m.items_[all[i]->id()] = all[i]->state();
    for (std::map<std::string, Participant>::iterator it = participants_.begin(); it != participants_.end(); ++it)
        m.workloads_[it->first] = it->second.workload();
    m.data_ = data_;
    m.escalationCount_ = escalationCount_;
    m.escalatedTo_ = escalatedTo_;
    m.escalationReason_ = escalationReason_;
    return m;
}

void WorkflowInstance::restore(const WorkflowMemento& m) {
    std::vector<WorkflowComponent*> all = items();
    for (size_t i = 0; i < all.size(); ++i) {
        std::map<std::string, ItemState>::const_iterator it = m.items_.find(all[i]->id());
        if (it != m.items_.end()) all[i]->setState(it->second);
    }
    for (std::map<std::string, int>::const_iterator it = m.workloads_.begin(); it != m.workloads_.end(); ++it) {
        Participant* p = findParticipant(it->first);
        if (p) p->setWorkload(it->second);
    }
    data_ = m.data_;
    escalationCount_ = m.escalationCount_;
    escalatedTo_ = m.escalatedTo_;
    escalationReason_ = m.escalationReason_;
    state_ = &WorkflowState::byName(m.stateName_);
}

void WorkflowInstance::recordEvent(EventType type, const std::string& subject, const std::string& actor,
                                   const std::string& recipient, const std::string& message) {
    WorkflowEvent e;
    e.type = type;
    e.instanceId = id_;
    e.subject = subject;
    e.actor = actor;
    e.recipient = recipient;
    e.message = message;
    notify(e);
}

// ---------------------------------------------------------------- implementations used by states

WorkflowComponent& WorkflowInstance::requireItem(const std::string& itemId) {
    WorkflowComponent* item = findItem(itemId);
    if (!item) throw WorkflowError("Unknown work item '" + itemId + "'");
    return *item;
}

void WorkflowInstance::transitionTo(WorkflowState& next, const std::string& actor, const std::string& reason) {
    std::string from = state_->name();
    state_ = &next;
    recordEvent(EventType::StateChanged, "", actor, "", from + " -> " + next.name() + (reason.empty() ? "" : " (" + reason + ")"));
}

void WorkflowInstance::doStart(const std::string& actor) {
    recordEvent(EventType::InstanceStarted, "", actor, "", "Workflow started");
    advance(actor);
}

void WorkflowInstance::assignTo(WorkflowComponent& item, Participant& p, const std::string& actor) {
    item.assign(p.name());
    p.adjustWorkload(1);
    recordEvent(EventType::WorkAssigned, item.id(), actor, p.name(),
                "'" + item.displayName() + "' assigned to " + p.name());
}

void WorkflowInstance::release(WorkflowComponent& item) {
    if (item.status() != WorkStatus::Assigned) return;
    Participant* p = findParticipant(item.assignee());
    if (p) p->adjustWorkload(-1);
}

void WorkflowInstance::doAssign(const std::string& itemId, const std::string& participant, const std::string& actor) {
    WorkflowComponent& item = requireItem(itemId);
    if (item.status() != WorkStatus::Available && item.status() != WorkStatus::Assigned)
        throw InvalidTransition("Cannot assign '" + itemId + "': it is " + statusName(item.status()));

    Participant* chosen = 0;
    if (participant.empty()) {
        chosen = definition_->strategy().select(item, participants());
        if (!chosen) throw WorkflowError("Nobody with role '" + item.requiredRole() + "' is available for '" + itemId + "'");
    } else {
        chosen = findParticipant(participant);
        if (!chosen) throw WorkflowError("Unknown participant '" + participant + "'");
        if (chosen->role().name() != item.requiredRole())
            throw WorkflowError(participant + " has role '" + chosen->role().name() + "' but '" + itemId +
                                "' needs '" + item.requiredRole() + "'");
        if (!chosen->isAvailable()) throw WorkflowError(participant + " is not available");
    }
    release(item);  // re-assignment frees the previous owner
    assignTo(item, *chosen, actor);
    state_->onWorkAssigned(*this);
}

void WorkflowInstance::doComplete(const std::string& itemId, const std::string& actor, const std::string& outcome) {
    WorkflowComponent& item = requireItem(itemId);
    if (item.status() != WorkStatus::Assigned)
        throw InvalidTransition("Cannot complete '" + itemId + "': it is " + statusName(item.status()));
    if (item.assignee() != actor)
        throw WorkflowError("'" + itemId + "' is assigned to " + item.assignee() + ", not " + actor);
    std::string result = outcome.empty() ? "done" : outcome;
    release(item);
    item.complete(result);
    recordEvent(EventType::WorkCompleted, itemId, actor, "", "'" + item.displayName() + "' completed with outcome '" + result + "'");
    advance(actor);
}

void WorkflowInstance::doCancelItem(const std::string& itemId, const std::string& actor, const std::string& reason) {
    WorkflowComponent& item = requireItem(itemId);
    if (isTerminal(item.status()))
        throw InvalidTransition("Cannot cancel '" + itemId + "': it is already " + statusName(item.status()));
    release(item);
    item.cancel();
    recordEvent(EventType::WorkCancelled, itemId, actor, "", "'" + item.displayName() + "' cancelled: " + reason);
    advance(actor);
}

void WorkflowInstance::doEscalate(const std::string& actor, const std::string& reason) {
    const Handler* chain = definition_->escalationChain();
    if (!chain) throw WorkflowError("'" + definition_->name() + "' has no escalation chain");
    EscalationRequest request;
    request.instanceId = id_;
    request.reason = reason;
    request.requiredLevel = escalationCount_ + 1;
    request.participants = participants();
    if (!chain->handle(request)) {
        std::string trail;
        for (size_t i = 0; i < request.trail.size(); ++i) trail += (i ? "; " : "") + request.trail[i];
        throw WorkflowError("Escalation failed, nobody can take it (" + trail + ")");
    }
    ++escalationCount_;
    escalatedTo_ = request.handledBy->name();
    escalationReason_ = reason;
    transitionTo(EscalatedState::instance(), actor, "escalation " + std::to_string(escalationCount_));
    std::string trail;
    for (size_t i = 0; i < request.trail.size(); ++i) trail += (i ? "; " : "") + request.trail[i];
    recordEvent(EventType::Escalated, "", actor, escalatedTo_, "Escalated to " + escalatedTo_ + ": " + reason + " [" + trail + "]");
}

void WorkflowInstance::doResolveEscalation(const std::string& actor, const std::string& notes) {
    if (actor != escalatedTo_) throw WorkflowError("Only " + escalatedTo_ + " can resolve this escalation, not " + actor);
    std::string handler = escalatedTo_;
    escalatedTo_.clear();
    recordEvent(EventType::EscalationResolved, "", actor, "", handler + " resolved the escalation: " + notes);
    transitionTo(InProgressState::instance(), actor, "escalation resolved");
}

void WorkflowInstance::doClose(const std::string& actor, const std::string& reason) {
    std::vector<WorkflowComponent*> all = items();
    for (size_t i = 0; i < all.size(); ++i) {
        if (isTerminal(all[i]->status())) continue;
        release(*all[i]);
        all[i]->cancel();
        recordEvent(EventType::WorkCancelled, all[i]->id(), actor, "", "Cancelled because the workflow was closed");
    }
    escalatedTo_.clear();
    recordEvent(EventType::WorkflowClosed, "", actor, "", reason);
    transitionTo(ClosedState::instance(), actor, "closed");
}

// ---------------------------------------------------------------- routing engine

/**
 * Decide what happens to every Pending item. Repeats until nothing changes, because skipping or
 * cancelling one item can settle the items that depend on it.
 *   - a cancelled dependency cancels the item
 *   - unfinished dependency: keep waiting
 *   - routing condition not met (or its source skipped): the item is skipped (branch not taken)
 *   - otherwise the item becomes Available and the strategy tries to assign it
 */
void WorkflowInstance::advance(const std::string& actor) {
    bool assignedSomething = false;
    bool changed = true;
    while (changed) {
        changed = false;
        std::vector<WorkflowComponent*> all = items();
        for (size_t i = 0; i < all.size(); ++i) {
            WorkflowComponent& item = *all[i];
            if (item.status() != WorkStatus::Pending) continue;

            bool waiting = false, blocked = false;
            const std::vector<std::string>& deps = item.dependencies();
            for (size_t d = 0; d < deps.size(); ++d) {
                WorkflowComponent* dep = findItem(deps[d]);
                WorkStatus s = dep ? dep->status() : WorkStatus::Cancelled;
                if (s == WorkStatus::Cancelled) blocked = true;
                else if (s != WorkStatus::Completed && s != WorkStatus::Skipped) waiting = true;
            }
            if (blocked) {
                item.cancel();
                recordEvent(EventType::WorkCancelled, item.id(), actor, "", "Cancelled because a dependency was cancelled");
                changed = true;
                continue;
            }
            if (waiting) continue;

            if (item.hasCondition()) {
                WorkflowComponent* src = findItem(item.conditionSource());
                bool taken = src && src->status() == WorkStatus::Completed && src->outcome() == item.conditionOutcome();
                if (!taken) {
                    item.skip();
                    recordEvent(EventType::RoutingDecision, item.id(), actor, "",
                                "Branch '" + item.id() + "' not taken: '" + item.conditionSource() + "' did not end with '" +
                                    item.conditionOutcome() + "'");
                    recordEvent(EventType::WorkSkipped, item.id(), actor, "", "'" + item.displayName() + "' skipped");
                    changed = true;
                    continue;
                }
                recordEvent(EventType::RoutingDecision, item.id(), actor, "",
                            "Branch '" + item.id() + "' taken: '" + item.conditionSource() + "' ended with '" +
                                item.conditionOutcome() + "'");
            }

            item.makeAvailable();
            recordEvent(EventType::WorkAvailable, item.id(), actor, "", "'" + item.displayName() + "' is available");
            Participant* p = definition_->strategy().select(item, participants());
            if (p) {
                assignTo(item, *p, "system");
                assignedSomething = true;
            }
            changed = true;
        }
    }
    if (assignedSomething) state_->onWorkAssigned(*this);
    if (allWorkFinished()) state_->onAllWorkFinished(*this);
}

} // namespace tividy
