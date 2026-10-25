#include "engine/WorkItem.h"
#include <algorithm>

namespace tividy {

WorkItem::WorkItem(const std::string& id, const std::string& name, const std::string& requiredRole)
    : WorkflowComponent(id, name), role_(requiredRole) {}

WorkflowComponent* WorkItem::clone() const {
    WorkItem* copy = new WorkItem(id(), name(), role_);
    copy->deps_ = deps_;
    copy->condSource_ = condSource_;
    copy->condOutcome_ = condOutcome_;
    copy->state_ = state_;
    return copy;
}

void WorkItem::collectLeaves(std::vector<WorkflowComponent*>& out) { out.push_back(this); }

void WorkItem::addDependency(const std::string& itemId) {
    if (std::find(deps_.begin(), deps_.end(), itemId) == deps_.end()) deps_.push_back(itemId);
}

void WorkItem::setCondition(const std::string& sourceId, const std::string& outcome) {
    condSource_ = sourceId;
    condOutcome_ = outcome;
    addDependency(sourceId);  // a conditional item always waits for its decision
}

void WorkItem::require(WorkStatus expected, const char* action) const {
    if (state_.status != expected)
        throw InvalidTransition(std::string("Cannot ") + action + " '" + id() + "': it is " + statusName(state_.status) +
                                ", expected " + statusName(expected));
}

void WorkItem::makeAvailable() {
    require(WorkStatus::Pending, "make available");
    state_.status = WorkStatus::Available;
}

void WorkItem::assign(const std::string& participant) {
    if (state_.status != WorkStatus::Available && state_.status != WorkStatus::Assigned)
        throw InvalidTransition("Cannot assign '" + id() + "': it is " + statusName(state_.status));
    state_.status = WorkStatus::Assigned;
    state_.assignee = participant;
}

void WorkItem::unassign() {
    require(WorkStatus::Assigned, "unassign");
    state_.status = WorkStatus::Available;
    state_.assignee.clear();
}

void WorkItem::complete(const std::string& outcome) {
    require(WorkStatus::Assigned, "complete");
    state_.status = WorkStatus::Completed;
    state_.outcome = outcome;
}

void WorkItem::skip() {
    require(WorkStatus::Pending, "skip");
    state_.status = WorkStatus::Skipped;
}

void WorkItem::cancel() {
    if (isTerminal(state_.status))
        throw InvalidTransition("Cannot cancel '" + id() + "': it is already " + statusName(state_.status));
    state_.status = WorkStatus::Cancelled;
}

} // namespace tividy
