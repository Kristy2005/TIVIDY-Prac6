#include "WorkflowStage.h"
#include "WorkItem.h"

namespace tividy {
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

WorkflowStage::~WorkflowStage(){
    if (!ownsComponents) {
        return;
    }

    for (std::vector<WorkflowComponent*>::iterator it = components.begin(); it != components.end();++it) {
        delete *it;
    }
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
bool WorkflowStage::replace(WorkflowComponent* oldComponent, WorkflowComponent* newComponent) {
    if (oldComponent == nullptr || newComponent == nullptr || newComponent == this) {
        return false;
    }
    for (std::vector<WorkflowComponent*>::size_type i = 0; i < components.size(); ++i) {
        if (components[i] == newComponent) {
            return false;
        }
    }
    for (std::vector<WorkflowComponent*>::size_type i = 0; i < components.size(); ++i) {
        if (components[i] == oldComponent) {
            components[i] = newComponent;
            return true;
        }
    }
    return false;
}

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

void WorkflowStage::collectWorkItems(std::vector<WorkItem*>& items) {
    for (std::vector<WorkflowComponent*>::iterator it = components.begin();
         it != components.end(); ++it) {
        (*it)->collectWorkItems(items);
    }
}

WorkflowComponent* WorkflowStage::clone() const
{
    WorkflowStage* copy = new WorkflowStage();

    copy->ownsComponents = true;

    for (std::vector<WorkflowComponent*>::const_iterator it = components.begin();it != components.end();++it) {

        if (*it != nullptr) {
            copy->add((*it)->clone());
        }
    }

    return copy;
}
}