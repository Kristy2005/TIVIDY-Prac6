#include "WorkflowInstance.h"

#include "WorkflowState.h"
#include "SubmittedState.h"
#include "ActiveState.h"
#include "InProgressState.h"
#include "WorkflowObserver.h"
#include "WorkItem.h"
#include "WorkflowComponent.h"
#include "WorkflowDefinition.h"
#include "Participant.h"

#include <algorithm>
#include <stdexcept>
#include <vector>

namespace tividy {

WorkflowInstance::WorkflowInstance(WorkflowDefinition* workflowDefinition)
    : currentState(nullptr), definition(workflowDefinition), workTree(nullptr) {
    static SubmittedState submitted;
    currentState = &submitted;

    if (definition != nullptr && definition->getRoot() != nullptr) {
        workTree = definition->createWorkTree();

        if (workTree != nullptr) {
            std::vector<WorkItem*> prototypeItems;
            std::vector<WorkItem*> instanceItems;
            definition->getRoot()->collectWorkItems(prototypeItems);
            workTree->collectWorkItems(instanceItems);

            const std::size_t count =
                prototypeItems.size() < instanceItems.size()
                    ? prototypeItems.size() : instanceItems.size();

            for (std::size_t i = 0; i < count; ++i) {
                prototypeToInstance[prototypeItems[i]] = instanceItems[i];
                workItemProgress.push_back(WorkItemProgress(instanceItems[i]));
            }
        }
    }

}

WorkflowInstance::~WorkflowInstance() {
    // Remove assignments from external participants before this instance's
    // address becomes invalid. This also keeps their workload counters valid.
    for (std::size_t i = 0; i < participants.size(); ++i) {
        if (participants[i] != nullptr) {
            participants[i]->clearAssignmentsForInstance(this);
        }
    }

    for (std::size_t i = 0; i < ownedParticipants.size(); ++i) {
        delete ownedParticipants[i];
    }
    ownedParticipants.clear();

    for (std::size_t i = 0; i < ownedWorkItems.size(); ++i) {
        delete ownedWorkItems[i];
    }
    ownedWorkItems.clear();

    delete workTree;
}

WorkItem* WorkflowInstance::resolveWorkItem(WorkItem* item) const {
    if (item == nullptr) {
        throw std::invalid_argument("WorkflowInstance: work item cannot be null");
    }

    std::map<const WorkItem*, WorkItem*>::const_iterator mapped =
        prototypeToInstance.find(item);
    WorkItem* actualItem = mapped == prototypeToInstance.end()
        ? item : mapped->second;

    for (std::size_t i = 0; i < workItemProgress.size(); ++i) {
        if (workItemProgress[i].item == actualItem) {
            return actualItem;
        }
    }

    throw std::invalid_argument(
        "WorkflowInstance: work item does not belong to this instance"
    );
}

void WorkflowInstance::ensureEditable() const {
    if (dynamic_cast<SubmittedState*>(currentState) == nullptr &&
        dynamic_cast<ActiveState*>(currentState) == nullptr &&
        dynamic_cast<InProgressState*>(currentState) == nullptr) {
        throw std::logic_error(
            "WorkflowInstance: changes are allowed only before the workflow is Resolved or Closed"
        );
    }
}

void WorkflowInstance::releaseAssignment(WorkItem* item) {
    std::map<WorkItem*, Participant*>::iterator assigned =
        assignedParticipants.find(item);
    if (assigned == assignedParticipants.end()) {
        return;
    }

    if (assigned->second != nullptr) {
        assigned->second->unassignWork(this, item);
    }
    assignedParticipants.erase(assigned);
}

void WorkflowInstance::completeWorkItem(WorkItem* item) {
    WorkItem* actualItem = resolveWorkItem(item);

    for (std::size_t i = 0; i < workItemProgress.size(); ++i) {
        if (workItemProgress[i].item == actualItem) {
            if (workItemProgress[i].cancelled) {
                throw std::logic_error("WorkflowInstance: a cancelled work item cannot be completed");
            }
            if (workItemProgress[i].completed) {
                throw std::logic_error("WorkflowInstance: work item is already completed");
            }

            releaseAssignment(actualItem);
            workItemProgress[i].completed = true;
            actualItem->complete();
            history.recordEvent("Work completed", actualItem->getName());
            return;
        }
    }

    // resolveWorkItem() already guarantees membership; this is defensive.
    throw std::invalid_argument(
        "WorkflowInstance: work item does not belong to this instance"
    );
}

bool WorkflowInstance::isWorkItemComplete(WorkItem* item) const {
    WorkItem* actualItem = resolveWorkItem(item);
    for (std::size_t i = 0; i < workItemProgress.size(); ++i) {
        if (workItemProgress[i].item == actualItem) {
            return workItemProgress[i].completed;
        }
    }
    return false;
}

bool WorkflowInstance::isWorkItemCancelled(WorkItem* item) const {
    WorkItem* actualItem = resolveWorkItem(item);
    for (std::size_t i = 0; i < workItemProgress.size(); ++i) {
        if (workItemProgress[i].item == actualItem) {
            return workItemProgress[i].cancelled;
        }
    }
    return false;
}

const WorkflowHistory& WorkflowInstance::getHistory() const {
    return history;
}

const std::vector<WorkItemProgress>&
WorkflowInstance::getWorkItemProgress() const {
    return workItemProgress;
}

WorkItem* WorkflowInstance::getInstanceWorkItem(WorkItem* item) const {
    return resolveWorkItem(item);
}

WorkItem* WorkflowInstance::addWorkItem(
    const std::string& name,
    const std::string& requiredRole,
    WorkItem* dependency) {
    ensureEditable();

    if (name.find_first_not_of(" \t\r\n") == std::string::npos) {
        throw std::invalid_argument("addWorkItem: work item name cannot be empty");
    }
    if (requiredRole.find_first_not_of(" \t\r\n") == std::string::npos) {
        throw std::invalid_argument("addWorkItem: required role cannot be empty");
    }

    for (std::size_t i = 0; i < workItemProgress.size(); ++i) {
        if (workItemProgress[i].item->getName() == name) {
            throw std::invalid_argument(
                "addWorkItem: a work item with that name already exists in this instance"
            );
        }
    }

    WorkItem* actualDependency = nullptr;
    if (dependency != nullptr) {
        actualDependency = resolveWorkItem(dependency);
        if (isWorkItemCancelled(actualDependency)) {
            throw std::invalid_argument(
                "addWorkItem: a cancelled item cannot be used as a dependency"
            );
        }
    }

    WorkItem* item = new WorkItem(name, requiredRole);
    if (actualDependency != nullptr) {
        item->addDependency(actualDependency);
    }

    try {
        ownedWorkItems.push_back(item);
        workItemProgress.push_back(WorkItemProgress(item));
    } catch (...) {
        if (!ownedWorkItems.empty() && ownedWorkItems.back() == item) {
            ownedWorkItems.pop_back();
        }
        delete item;
        throw;
    }

    history.recordEvent("Work item added", item->getName());
    return item;
}

void WorkflowInstance::cancelWorkItem(WorkItem* item) {
    ensureEditable();
    WorkItem* actualItem = resolveWorkItem(item);

    for (std::size_t i = 0; i < workItemProgress.size(); ++i) {
        if (workItemProgress[i].item == actualItem) {
            if (workItemProgress[i].completed) {
                throw std::logic_error("cancelWorkItem: completed work items cannot be cancelled");
            }
            if (workItemProgress[i].cancelled) {
                throw std::logic_error("cancelWorkItem: work item is already cancelled");
            }
        }
    }

    // Do not silently unblock work whose prerequisite is being cancelled.
    for (std::size_t i = 0; i < workItemProgress.size(); ++i) {
        const WorkItemProgress& candidate = workItemProgress[i];
        if (candidate.item == actualItem || candidate.completed || candidate.cancelled) {
            continue;
        }

        const std::vector<WorkItem*>& dependencies = candidate.item->getDependencies();
        for (std::size_t d = 0; d < dependencies.size(); ++d) {
            WorkItem* dependency = dependencies[d];
            std::map<const WorkItem*, WorkItem*>::const_iterator mapped =
                prototypeToInstance.find(dependency);
            if (mapped != prototypeToInstance.end()) {
                dependency = mapped->second;
            }
            if (dependency == actualItem) {
                throw std::logic_error(
                    "cancelWorkItem: another incomplete work item depends on this item"
                );
            }
        }
    }

    std::string participantName;
    Participant* assigned = getAssignee(actualItem);
    if (assigned != nullptr) {
        participantName = assigned->getName();
    }
    releaseAssignment(actualItem);

    for (std::size_t i = 0; i < workItemProgress.size(); ++i) {
        if (workItemProgress[i].item == actualItem) {
            workItemProgress[i].cancelled = true;
            history.recordEvent("Work cancelled", actualItem->getName(), participantName);
            return;
        }
    }
}

void WorkflowInstance::changeWorkItemRole(
    WorkItem* item,
    const std::string& requiredRole) {
    ensureEditable();
    WorkItem* actualItem = resolveWorkItem(item);
    if (isWorkItemComplete(actualItem) || isWorkItemCancelled(actualItem)) {
        throw std::logic_error("changeWorkItemRole: only active, incomplete work can be altered");
    }
    if (requiredRole.find_first_not_of(" \t\r\n") == std::string::npos) {
        throw std::invalid_argument("changeWorkItemRole: required role cannot be empty");
    }

    Participant* previousAssignee = getAssignee(actualItem);
    actualItem->setRequiredRole(requiredRole);
    if (previousAssignee != nullptr && !requiredRole.empty() &&
        !previousAssignee->hasRole(requiredRole)) {
        releaseAssignment(actualItem);
    }

    history.recordEvent(
        "Work role changed to " + (requiredRole.empty() ? std::string("any role") : requiredRole),
        actualItem->getName(),
        previousAssignee == nullptr ? std::string() : previousAssignee->getName()
    );
}

void WorkflowInstance::changeWorkItemPriority(WorkItem* item, int priority) {
    ensureEditable();
    WorkItem* actualItem = resolveWorkItem(item);
    if (isWorkItemComplete(actualItem) || isWorkItemCancelled(actualItem)) {
        throw std::logic_error("changeWorkItemPriority: only active, incomplete work can be altered");
    }
    if (priority < 0) {
        throw std::invalid_argument("changeWorkItemPriority: priority cannot be negative");
    }

    actualItem->setPriority(priority);
    history.recordEvent(
        "Work priority changed to " + std::to_string(priority),
        actualItem->getName()
    );
}

void WorkflowInstance::addParticipant(Participant* participant, bool takeOwnership) {
    ensureEditable();
    if (participant == nullptr) {
        throw std::invalid_argument("addParticipant: participant cannot be null");
    }
    if (participant->getName().find_first_not_of(" \t\r\n") == std::string::npos) {
        throw std::invalid_argument("addParticipant: participant name cannot be empty");
    }

    bool alreadyRegistered = false;
    for (std::size_t i = 0; i < participants.size(); ++i) {
        if (participants[i] == participant) {
            alreadyRegistered = true;
            break;
        }
        if (participants[i] != nullptr &&
            participants[i]->getName() == participant->getName()) {
            throw std::invalid_argument(
                "addParticipant: a participant with that name already exists in this instance"
            );
        }
    }

    if (!alreadyRegistered) {
        participants.push_back(participant);
        history.recordEvent("Participant added", std::string(), participant->getName());
    }

    if (takeOwnership &&
        std::find(ownedParticipants.begin(), ownedParticipants.end(), participant)
            == ownedParticipants.end()) {
        ownedParticipants.push_back(participant);
    }
}

const std::vector<Participant*>& WorkflowInstance::getParticipantPointers() const {
    return participants;
}

void WorkflowInstance::assignParticipant(WorkItem* item, Participant* participant) {
    ensureEditable();
    WorkItem* actualItem = resolveWorkItem(item);
    if (participant == nullptr) {
        throw std::invalid_argument("assignParticipant: participant cannot be null");
    }
    if (isWorkItemComplete(actualItem) || isWorkItemCancelled(actualItem)) {
        throw std::logic_error("assignParticipant: only active, incomplete work can be assigned");
    }
    if (!areDependenciesComplete(actualItem)) {
        throw std::logic_error("assignParticipant: work item dependencies are not complete");
    }
    if (!actualItem->getRequiredRole().empty() &&
        !participant->hasRole(actualItem->getRequiredRole())) {
        throw std::invalid_argument(
            "assignParticipant: participant does not have the required role '" +
            actualItem->getRequiredRole() + "'"
        );
    }

    addParticipant(participant, false);

    std::map<WorkItem*, Participant*>::iterator current =
        assignedParticipants.find(actualItem);
    if (current != assignedParticipants.end() && current->second == participant) {
        return;
    }

    const bool wasAssigned = current != assignedParticipants.end();
    releaseAssignment(actualItem);
    participant->assignWork(this, actualItem);
    assignedParticipants[actualItem] = participant;
    history.recordEvent(
        wasAssigned ? "Work reassigned" : "Work assigned",
        actualItem->getName(),
        participant->getName()
    );
}

Participant* WorkflowInstance::getAssignee(WorkItem* item) const {
    WorkItem* actualItem = resolveWorkItem(item);
    std::map<WorkItem*, Participant*>::const_iterator assigned =
        assignedParticipants.find(actualItem);
    return assigned == assignedParticipants.end() ? nullptr : assigned->second;
}

WorkflowState* WorkflowInstance::getState() const {
    return currentState;
}

WorkflowDefinition* WorkflowInstance::getDefinition() const {
    return definition;
}

bool WorkflowInstance::areDependenciesComplete(WorkItem* item) const {
    WorkItem* actualItem = resolveWorkItem(item);
    const std::vector<WorkItem*>& dependencies = actualItem->getDependencies();

    for (std::size_t i = 0; i < dependencies.size(); ++i) {
        WorkItem* dependency = dependencies[i];
        std::map<const WorkItem*, WorkItem*>::const_iterator mapped =
            prototypeToInstance.find(dependency);
        if (mapped != prototypeToInstance.end()) {
            dependency = mapped->second;
        }

        if (isWorkItemCancelled(dependency) || !isWorkItemComplete(dependency)) {
            return false;
        }
    }
    return true;
}

bool WorkflowInstance::areAllWorkItemsComplete() const {
    bool hasActiveWork = false;
    for (std::size_t i = 0; i < workItemProgress.size(); ++i) {
        if (workItemProgress[i].cancelled) {
            continue;
        }
        hasActiveWork = true;
        if (!workItemProgress[i].completed) {
            return false;
        }
    }
    return hasActiveWork;
}

void WorkflowInstance::setState(WorkflowState* state) {
    if (state == nullptr) {
        throw std::invalid_argument("WorkflowInstance: state cannot be null");
    }
    if (state == currentState) {
        return;
    }
    currentState = state;
    notifyObservers();
}

void WorkflowInstance::submit() {
    // New instances begin in SubmittedState; keep submission in their audit log.
    history.recordEvent("Workflow instance submitted");
}

void WorkflowInstance::activate() {
    currentState->activate(this);
    history.recordEvent("Workflow activated");
}

void WorkflowInstance::startProcessing() {
    currentState->startProcessing(this);
    history.recordEvent("Workflow processing started");
}

void WorkflowInstance::escalate() {
    if (definition == nullptr) {
        throw std::logic_error("Workflow definition is missing.");
    }
    if (dynamic_cast<InProgressState*>(currentState) == nullptr) {
        throw std::logic_error("Only in-progress workflows can be escalated.");
    }
    definition->handleEscalation(this);
    history.recordEvent("Workflow escalated");
}

WorkflowMemento* WorkflowInstance::createMemento() {
    WorkflowMemento* memento = new WorkflowMemento();
    memento->setState(currentState);
    return memento;
}

void WorkflowInstance::restore(WorkflowMemento* memento) {
    if (memento == nullptr || memento->getState() == nullptr) {
        throw std::invalid_argument("Cannot restore from an invalid memento.");
    }
    setState(memento->getState());
}

void WorkflowInstance::resolve() {
    currentState->resolve(this);
    history.recordEvent("Workflow resolved");
}

void WorkflowInstance::close() {
    currentState->close(this);
    history.recordEvent("Workflow closed");
}

void WorkflowInstance::notifyObservers() {
    const std::vector<WorkflowObserver*> snapshot = observers;
    for (std::size_t i = 0; i < snapshot.size(); ++i) {
        WorkflowObserver* observer = snapshot[i];
        if (observer != nullptr &&
            std::find(observers.begin(), observers.end(), observer) != observers.end()) {
            observer->update(this);
        }
    }
}

void WorkflowInstance::attach(WorkflowObserver* observer) {
    if (observer == nullptr) {
        throw std::invalid_argument("Cannot attach a null observer.");
    }
    if (std::find(observers.begin(), observers.end(), observer) == observers.end()) {
        observers.push_back(observer);
    }
}

void WorkflowInstance::detach(WorkflowObserver* observer) {
    observers.erase(
        std::remove(observers.begin(), observers.end(), observer),
        observers.end()
    );
}

const std::vector<std::string> WorkflowInstance::getParticipants() const {
    std::vector<std::string> participantListAndRoles;
    for (std::size_t i = 0; i < participants.size(); ++i) {
        if (participants[i] == nullptr) {
            continue;
        }
        participantListAndRoles.push_back(participants[i]->getName());
        const std::vector<std::string>& roles = participants[i]->getRoles();
        for (std::size_t r = 0; r < roles.size(); ++r) {
            participantListAndRoles.push_back(roles[r]);
        }
    }
    return participantListAndRoles;
}

} // namespace tividy
