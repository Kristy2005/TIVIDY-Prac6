#include "SubProcess.h"
#include "WorkItem.h"

namespace tividy {
void SubProcess::add(WorkflowComponent* component) {
    if (component == nullptr || component == this) {
        return;
    }
    for (std::vector<WorkflowComponent*>::size_type i = 0; i < components.size(); ++i) {
        if (components[i] == component) {
            return; // already present, so don't add a duplicate
        }
    }
    components.push_back(component);
}

SubProcess::~SubProcess(){
    if (!ownsComponents) {
        return;
    }

    for (std::vector<WorkflowComponent*>::iterator it = components.begin();
         it != components.end();
         ++it) {
        delete *it;
    }
}

void SubProcess::remove(WorkflowComponent* component) {
    if (component == nullptr) {
        return;
    }
    for (std::vector<WorkflowComponent*>::size_type i = 0; i < components.size(); ++i) {
        if (components[i] == component) {
            components.erase(components.begin() + i);
            return; // add() prevents duplicates, so there is only one match
        }
    }
}
void SubProcess::execute() {
    for (std::vector<WorkflowComponent*>::iterator it = components.begin();
         it != components.end(); ++it) {
        (*it)->execute();
    }
}
bool SubProcess::isComplete() const {
    for (std::vector<WorkflowComponent*>::const_iterator it = components.begin();
         it != components.end(); ++it) {
        if (!(*it)->isComplete()) {
            return false;
        }
    }
    return true;
}

void SubProcess::collectWorkItems(std::vector<WorkItem*>& items) {
    for (std::vector<WorkflowComponent*>::iterator it = components.begin();
         it != components.end(); ++it) {
        (*it)->collectWorkItems(items);
    }
}

WorkflowComponent* SubProcess::clone() const
{
    SubProcess* copy = new SubProcess();

    copy->ownsComponents = true;

    for (std::vector<WorkflowComponent*>::const_iterator it = components.begin();it != components.end();++it) {

        if (*it != nullptr) {
            copy->add((*it)->clone());
        }
    }

    return copy;
}
}
