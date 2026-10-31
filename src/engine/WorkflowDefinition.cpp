#include "WorkflowDefinition.h"
#include "WorkflowInstance.h"
#include "Participant.h"
#include "WorkItem.h"

#include <stdexcept>

namespace tividy {

WorkflowDefinition::WorkflowDefinition()
: name(),
root(nullptr),
strategy(nullptr),
escalationHandler(nullptr) {
}

void WorkflowDefinition::setRoot(WorkflowComponent* component) {
    root = component;
}

WorkflowComponent* WorkflowDefinition::getRoot() const {
    return root;
}

void WorkflowDefinition::setAssignmentStrategy(
AssignmentStrategy* strategyValue) {
strategy = strategyValue;
}

AssignmentStrategy* WorkflowDefinition::getAssignmentStrategy() const {
return strategy;
}

Participant* WorkflowDefinition::selectAssignee(
WorkItem* item,
std::vector<Participant*> participants) const {

 
if (strategy == nullptr) {
    return nullptr;
}

return strategy->assign(item, participants);
 

}

void WorkflowDefinition::setEscalationHandler(Handler* handler) {
escalationHandler = handler;
}

WorkflowInstance* WorkflowDefinition::createInstance() {
return new WorkflowInstance(this);
}

void WorkflowDefinition::handleEscalation(WorkflowInstance* instance) {
    if (instance == nullptr) {
        throw std::invalid_argument(
            "WorkflowDefinition: instance cannot be null"
        );
    }

    if (escalationHandler == nullptr) {
        throw std::logic_error(
            "WorkflowDefinition: no escalation handler configured"
        );
    }

    escalationHandler->handleRequest(instance);
}

Participant* WorkflowDefinition::selectAssignee(
    WorkflowInstance* instance,
    WorkItem* item,
    std::vector<Participant*> participants
) const {

    if (strategy == nullptr || instance == nullptr || item == nullptr) {
        return nullptr;
    }

    return strategy->assign(instance, item, participants);
}

WorkflowComponent* WorkflowDefinition::createWorkTree() const {
    if (root == nullptr) {
        return nullptr;
    }

    return root->clone();
}
}
