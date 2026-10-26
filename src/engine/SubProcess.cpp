#include "SubProcess.h"

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
}
