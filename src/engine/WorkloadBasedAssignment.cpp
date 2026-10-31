#include "WorkloadBasedAssignment.h"
#include "Participant.h"
#include "WorkItem.h"
#include "WorkflowInstance.h"

#include <stdexcept>

namespace tividy {

// Existing assignment method
Participant* WorkloadBasedAssignment::assign(
    WorkItem* item,
    std::vector<Participant*> participants) {

    if (item == nullptr || item->isComplete()) {
        return nullptr;
    }

    Participant* selected = nullptr;

    for (std::vector<Participant*>::iterator it = participants.begin();
         it != participants.end(); ++it) {

        Participant* candidate = *it;

        if (candidate == nullptr) {
            continue;
        }

        if (selected == nullptr ||
            candidate->getWorkload() < selected->getWorkload()) {
            selected = candidate;
        }
    }

    return selected;
}


// New instance-aware assignment method
Participant* WorkloadBasedAssignment::assign(
    WorkflowInstance* instance,
    WorkItem* item,
    std::vector<Participant*> participants) {

    if (instance == nullptr || item == nullptr) {
        return nullptr;
    }

    // Check completion within this particular workflow instance.
    try {
        if (instance->isWorkItemComplete(item)) {
            return nullptr;
        }
    }
    catch (const std::invalid_argument&) {
        // The work item does not belong to this instance.
        return nullptr;
    }

    Participant* selected = nullptr;

    for (std::vector<Participant*>::iterator it = participants.begin();
         it != participants.end(); ++it) {

        Participant* candidate = *it;

        if (candidate == nullptr) {
            continue;
        }

        if (selected == nullptr ||
            candidate->getWorkload() < selected->getWorkload()) {
            selected = candidate;
        }
    }

    return selected;
}

}