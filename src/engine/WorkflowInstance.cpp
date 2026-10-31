#include "WorkflowInstance.h"
#include "WorkflowState.h"
#include "SubmittedState.h"
#include "InProgressState.h"
#include "WorkflowObserver.h"
#include "WorkItem.h"
#include "WorkflowComponent.h"
#include "WorkflowDefinition.h"

#include <algorithm>
#include <stdexcept>
#include <vector>

namespace tividy {

WorkflowInstance::WorkflowInstance(WorkflowDefinition* definition)
    : currentState(nullptr), definition(definition) {

    static SubmittedState s;
    currentState = &s;

    if (this->definition != nullptr &&
        this->definition->getRoot() != nullptr) {

        std::vector<WorkItem*> items;
        this->definition->getRoot()->collectWorkItems(items);

        for (std::size_t i = 0; i < items.size(); ++i) {
            workItemProgress.push_back(WorkItemProgress(items[i]));
        }
    }
}

void WorkflowInstance::completeWorkItem(WorkItem* item) {
    if (item == nullptr) {
        throw std::invalid_argument(
            "WorkflowInstance: work item cannot be null"
        );
    }

    for (std::size_t i = 0; i < workItemProgress.size(); ++i) {
        if (workItemProgress[i].item == item) {
            workItemProgress[i].completed = true;
            return;
        }
    }

    throw std::invalid_argument(
        "WorkflowInstance: work item does not belong to this instance"
    );
}

bool WorkflowInstance::isWorkItemComplete(WorkItem* item) const {
    if (item == nullptr) {
        throw std::invalid_argument(
            "WorkflowInstance: work item cannot be null"
        );
    }

    for (std::size_t i = 0; i < workItemProgress.size(); ++i) {
        if (workItemProgress[i].item == item) {
            return workItemProgress[i].completed;
        }
    }

    throw std::invalid_argument(
        "WorkflowInstance: work item does not belong to this instance"
    );
}

const std::vector<WorkItemProgress>&
WorkflowInstance::getWorkItemProgress() const {
    return workItemProgress;
}

WorkflowInstance::~WorkflowInstance() {
    // delete currentState;
}

WorkflowState* WorkflowInstance::getState() const {
    return currentState;
}

WorkflowDefinition* WorkflowInstance::getDefinition() const {
    return definition;
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
    // New instances begin in SubmittedState.
}

void WorkflowInstance::activate() {
    currentState->activate(this);
}

void WorkflowInstance::startProcessing() {
    currentState->startProcessing(this);
}

void WorkflowInstance::escalate() {
    if (definition == nullptr) throw std::logic_error("Workflow definition is missing.");
    if (dynamic_cast<InProgressState*>(currentState) == nullptr)
        throw std::logic_error("Only in-progress workflows can be escalated.");
    definition->handleEscalation(this);
}

WorkflowMemento* WorkflowInstance::createMemento() {
    WorkflowMemento* memento = new WorkflowMemento();
    memento->setState(currentState);   // the memento gets its own copy
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
}

void WorkflowInstance::close() {
    currentState->close(this);
}

void WorkflowInstance::notifyObservers() {
    const std::vector<WorkflowObserver*> snapshot = observers;

    for (WorkflowObserver* observer : snapshot) {
        if (observer != nullptr &&
            std::find(observers.begin(), observers.end(), observer)
                != observers.end()) {
            observer->update(this);
        }
    }
}

void WorkflowInstance::attach(WorkflowObserver* observer) {
    if (observer == nullptr) {
        throw std::invalid_argument(
            "Cannot attach a null observer."
        );
    }

    if (std::find(observers.begin(), observers.end(), observer)
        == observers.end()) {
        observers.push_back(observer);
    }
}

void WorkflowInstance::detach(WorkflowObserver* observer) {
    observers.erase(
        std::remove(observers.begin(), observers.end(), observer),
        observers.end()
    );
}

}