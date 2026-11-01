#ifndef PARTICIPANT_H
#define PARTICIPANT_H

#include <string>
#include <vector>

#include "Role.h"
#include "WorkItem.h"

namespace tividy {

class WorkflowInstance;

class Participant {
public:
    Participant();
    explicit Participant(const std::string& participantName);

    // Existing assignment methods
    void assignWork(WorkItem* item);
    void completeWork(WorkItem* item);

    // New instance-aware assignment methods
    void assignWork(WorkflowInstance* instance, WorkItem* item);
    void completeWork(WorkflowInstance* instance, WorkItem* item);

    /**
     * @brief Release an instance-specific assignment without completing it.
     * @return true if an assignment was removed; otherwise false.
     */
    bool unassignWork(WorkflowInstance* instance, WorkItem* item);

    /** @brief Remove all assignments associated with an instance being destroyed. */
    void clearAssignmentsForInstance(WorkflowInstance* instance);

    int getWorkload() const;
    const std::string& getName() const;

    void addRole(const std::string& roleName);

    template <typename T>
    void addRole(const Role<T>* role) {
        if (role != nullptr) {
            addRole(role->getName());
        }
    }

    bool hasRole(const std::string& roleName) const;

    const std::vector<std::string>& getRoles() const;

private:

    // Represents an assignment belonging to a particular workflow instance.
    struct AssignedWork {
        WorkflowInstance* instance;
        WorkItem* item;

        AssignedWork(WorkflowInstance* workflow, WorkItem* workItem)
            : instance(workflow), item(workItem) {}
    };

    std::string name;
    int workload;

    std::vector<AssignedWork> assignedItems;
    std::vector<std::string> roles;
};

}

#endif