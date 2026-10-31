#include "Participant.h"
#include "WorkflowInstance.h"

#include <stdexcept>

namespace tividy {

Participant::Participant()
    : name(), workload(0), assignedItems(), roles() {
}

Participant::Participant(const std::string& participantName)
    : name(participantName), workload(0), assignedItems(), roles() {
}



void Participant::assignWork(WorkItem* item) {

    if (item == nullptr || item->isComplete()) {
        return;
    }

    // Prevent duplicate legacy assignments.
    for (std::size_t i = 0; i < assignedItems.size(); ++i) {

        if (assignedItems[i].instance == nullptr &&
            assignedItems[i].item == item) {
            return;
        }
    }

    assignedItems.push_back(AssignedWork(nullptr, item));
    ++workload;
}


void Participant::completeWork(WorkItem* item) {

    if (item == nullptr) {
        return;
    }

    for (std::size_t i = 0; i < assignedItems.size(); ++i) {

        if (assignedItems[i].instance == nullptr &&
            assignedItems[i].item == item) {

            item->complete();

            assignedItems.erase(assignedItems.begin() + i);

            if (workload > 0) {
                --workload;
            }

            return;
        }
    }
}



void Participant::assignWork(
    WorkflowInstance* instance,
    WorkItem* item) {

    if (instance == nullptr || item == nullptr) {
        return;
    }

    // Check completion in this particular workflow instance.
    try {
        if (instance->isWorkItemComplete(item)) {
            return;
        }
    }
    catch (const std::invalid_argument&) {
        // The item does not belong to this workflow instance.
        return;
    }

    // Prevent duplicate assignments within the same instance.
    for (std::size_t i = 0; i < assignedItems.size(); ++i) {

        if (assignedItems[i].instance == instance &&
            assignedItems[i].item == item) {
            return;
        }
    }

    assignedItems.push_back(AssignedWork(instance, item));
    ++workload;
}


void Participant::completeWork(
    WorkflowInstance* instance,
    WorkItem* item) {

    if (instance == nullptr || item == nullptr) {
        return;
    }

    for (std::size_t i = 0; i < assignedItems.size(); ++i) {

        if (assignedItems[i].instance == instance &&
            assignedItems[i].item == item) {

            // Update this instance's progress, not the shared WorkItem.
            instance->completeWorkItem(item);

            assignedItems.erase(assignedItems.begin() + i);

            if (workload > 0) {
                --workload;
            }

            return;
        }
    }
}



int Participant::getWorkload() const {
    return workload;
}


const std::string& Participant::getName() const {
    return name;
}



void Participant::addRole(const std::string& roleName) {
    if (roleName.empty()) {
        return;
    }

    if (!hasRole(roleName)) {
        roles.push_back(roleName);
    }
}

bool Participant::hasRole(const std::string& roleName) const {
    if (roleName.empty()) {
        return false;
    }

    for (std::size_t i = 0; i < roles.size(); ++i) {
        if (roles[i] == roleName) {
            return true;
        }
    }

    return false;
}

}