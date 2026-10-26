#include "RoleBasedAssignment.h"

namespace tividy {

#include "Participant.h"
#include "WorkItem.h"

Participant* RoleBasedAssignment::assign(
    WorkItem* item, std::vector<Participant*> participants) {
    if (item == nullptr || item->isComplete()) {
        return nullptr;
    }

    const std::string& requiredRole = item->getRequiredRole();
    if (requiredRole.empty()) {
        return nullptr;
    }

    for (std::vector<Participant*>::const_iterator it = participants.begin();
         it != participants.end(); ++it) {
        Participant* candidate = *it;
        if (candidate != nullptr && candidate->hasRole(requiredRole)) {
            return candidate;
        }
    }

    return nullptr;
}
}
