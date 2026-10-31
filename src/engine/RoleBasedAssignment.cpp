#include "RoleBasedAssignment.h"
#include "Participant.h"
#include "WorkItem.h"
#include "WorkflowInstance.h"

#include <stdexcept>

namespace tividy {

// Existing assignment method
Participant* RoleBasedAssignment::assign(
    WorkItem* item,
    std::vector<Participant*> participants) {

    if (item == nullptr || item->isComplete()) {
        return nullptr;
    }

    const std::string& requiredRole = item->getRequiredRole();

    if (requiredRole.empty()) {
        return nullptr;
    }

    for (std::vector<Participant*>::iterator it = participants.begin();
         it != participants.end(); ++it) {

        Participant* candidate = *it;

        if (candidate != nullptr && candidate->hasRole(requiredRole)) {
            return candidate;
        }
    }

    return nullptr;
}


// New instance-aware assignment method
Participant* RoleBasedAssignment::assign(
    WorkflowInstance* instance,
    WorkItem* item,
    std::vector<Participant*> participants) {

    if (instance == nullptr || item == nullptr) {
        return nullptr;
    }

    // Check completion in this particular workflow instance.
    try {
        if (instance->isWorkItemComplete(item)) {
            return nullptr;
        }
    }
    catch (const std::invalid_argument&) {
        // The work item does not belong to this workflow instance.
        return nullptr;
    }

    const std::string& requiredRole = item->getRequiredRole();

    if (requiredRole.empty()) {
        return nullptr;
    }

    for (std::vector<Participant*>::iterator it = participants.begin();
         it != participants.end(); ++it) {

        Participant* candidate = *it;

        if (candidate != nullptr && candidate->hasRole(requiredRole)) {
            return candidate;
        }
    }

    return nullptr;
}

}