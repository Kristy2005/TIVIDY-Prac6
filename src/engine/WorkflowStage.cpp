#include "engine/WorkflowStage.h"

void WorkflowStage::add(WorkflowComponent* component) {
    if (component == nullptr || component == this) {
        return;
    }

    for (std::vector<WorkflowComponent*>::size_type i = 0; i < components.size(); ++i) {
        if (components[i] == component) {
            return; 
        }
    }

    components.push_back(component);
}

void WorkflowStage::remove(WorkflowComponent* component) {
    if (component == nullptr) {
        return;
    }

    for (std::vector<WorkflowComponent*>::size_type i = 0; i < components.size(); ++i) {
        if (components[i] == component) {
            components.erase(components.begin() + i);
            return;
    }
}}

void WorkflowStage::execute() {
    for (std::vector<WorkflowComponent*>::iterator it = components.begin();
         it != components.end(); ++it) {
        (*it)->execute();
    }
}

bool WorkflowStage::isComplete() const {
    for (std::vector<WorkflowComponent*>::const_iterator it = components.begin();
         it != components.end(); ++it) {
        if (!(*it)->isComplete()) {
            return false;
        }
    }
    return true;
}