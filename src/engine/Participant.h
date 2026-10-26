#ifndef PARTICIPANT_H
#define PARTICIPANT_H

#include <string>
#include <vector>

#include "Role.h"
#include "WorkItem.h"

/**
 * @brief Person or service that can be assigned workflow work.
 *
 * A participant tracks the number of outstanding work items assigned to them
 * and the names of the roles they can perform. WorkItem and Role objects are
 * not owned by Participant; role names are copied so that a Participant does
 * not retain pointers to short-lived Role objects.
 */
class Participant {
public:
    /** @brief Construct a participant with an empty name and no workload. */
    Participant();

    /**
     * @brief Construct a participant with a name and no workload.
     * @param participantName Human-readable participant name.
     */
    explicit Participant(const std::string& participantName);

    /**
     * @brief Record an assignment if the work item is valid and not already assigned.
     * @param item Work item assigned to this participant; null is ignored.
     */
    void assignWork(WorkItem* item);

    /**
     * @brief Complete an outstanding item assigned to this participant.
     *
     * The item's completion state is updated and the participant's workload is
     * reduced only when this participant currently tracks the item.
     *
     * @param item Work item to complete; null or unassigned items are ignored.
     */
    void completeWork(WorkItem* item);

    /** @brief Return the number of outstanding assigned work items. */
    int getWorkload() const;

    /** @brief Return the participant's name. */
    const std::string& getName() const;

    /**
     * @brief Add a role by its name.
     * @param roleName Role name; duplicate names are ignored.
     */
    void addRole(const std::string& roleName);

    /**
     * @brief Add a named Role<T> to this participant.
     *
     * Only the role name is retained, avoiding a lifetime dependency on the
     * supplied Role object.
     *
     * @tparam T Role's existing template parameter.
     * @param role Role to add; null is ignored.
     */
    template <typename T>
    void addRole(const Role<T>* role) {
        if (role != nullptr) {
            addRole(role->getName());
        }
    }

    /**
     * @brief Check whether this participant can perform a named role.
     * @param roleName Role required by the work item.
     * @return true when the participant has the exact role name.
     */
    bool hasRole(const std::string& roleName) const;

private:
    std::string name;
    int workload;
    std::vector<WorkItem*> assignedItems;
    std::vector<std::string> roles;
};

#endif // TIVIDY_PARTICIPANT_H
