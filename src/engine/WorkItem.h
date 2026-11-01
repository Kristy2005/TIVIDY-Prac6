#ifndef TIVIDY_WORK_ITEM_H
#define TIVIDY_WORK_ITEM_H

#include <string>
#include <vector>
#include "WorkflowComponent.h"

/**
 * @brief A single, leaf work item in a workflow.
 *
 * WorkItem records whether a task has been completed. TIVIDY coordinates the
 * task but does not perform the real-world work; completion is recorded
 * explicitly with complete(). The optional required role is used by
 * RoleBasedAssignment to route work without hard-coding roles in the strategy.
 * 
 */

namespace tividy {
class WorkItem : public WorkflowComponent {
public:

    /**
     * @brief Adds another work item that must be completed before this item can proceed.
     * @param item Work item that this item depends on.
     */
    void addDependency(WorkItem* item);

    /**
     * @brief Returns the work items that must be completed before this item can proceed.
     * @return List of dependency work items.
     */
    const std::vector<WorkItem*>& getDependencies() const;

    /** @brief Construct an unnamed, incomplete work item. */
    WorkItem();

    /**
     * @brief Construct an incomplete work item with a descriptive name.
     * @param itemName Task name.
     */
    explicit WorkItem(const std::string& itemName);

    /**
     * @brief Construct a work item with a required participant role.
     * @param itemName Task name.
     * @param requiredRoleName Role that should receive this item.
     */
    WorkItem(const std::string& itemName, const std::string& requiredRoleName);

    /** @brief Execute the component interface without completing the task. */
    virtual void execute();

    /**
     * @brief Report whether the work item has been completed.
     * @return true after complete() has been called; otherwise false.
     */
    virtual bool isComplete() const;
    virtual void collectWorkItems(std::vector<WorkItem*>& items);

    /** @brief Mark this work item as complete. */
    void complete();

    /** @brief Return the name of this work item. */
    const std::string& getName() const;

    /** @brief Set the role required to perform this work item. */
    void setRequiredRole(const std::string& requiredRoleName);

    /** @brief Return the role required to perform this work item, or an empty string. */
    const std::string& getRequiredRole() const;

    /**
     * @brief Creates an independent copy of this work item.
     */
    WorkflowComponent* clone() const override;

private:
    /** @brief Task name. */
    std::string name;

    /** @brief Whether complete() has been called. */
    bool completed;

    /** @brief Required participant role name, or empty if unrestricted. */
    std::string requiredRole;

    /**
     * @brief Work items that must be completed before this item can proceed.
     */
    std::vector<WorkItem*> dependencies;
};


}

#endif // TIVIDY_WORK_ITEM_H
