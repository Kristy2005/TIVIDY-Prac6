#include "WorkflowInstance.h"
#include "WorkflowState.h"
#include "SubmittedState.h"
#include "InProgressState.h"
#include "WorkflowObserver.h"

#include <algorithm>
#include <stdexcept>

namespace tividy {

WorkflowInstance::WorkflowInstance(WorkflowDefinition* definition)
    : currentState(nullptr), definition(definition) {
    currentState = new SubmittedState();
}

WorkflowInstance::~WorkflowInstance() {
    delete currentState;
}

WorkflowState* WorkflowInstance::getState() const {
    return currentState;
}

void WorkflowInstance::setState(WorkflowState* state) {
    if (state == nullptr) {
        throw std::invalid_argument("WorkflowInstance: state cannot be null");
    }

    if (state == currentState) {
        return;
    }
    
    WorkflowState* old = currentState;
    currentState = state;
    delete old;              // safe: states have no data and don't touch `this` after setState()
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
    memento->setState(currentState->clone());   // the memento gets its own copy
    return memento;
}

void WorkflowInstance::restore(WorkflowMemento* memento) {
    if (memento == nullptr || memento->getState() == nullptr) {
        throw std::invalid_argument("Cannot restore from an invalid memento.");
    }
    setState(memento->getState()->clone());     // deletes the old state and notifies
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