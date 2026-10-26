#ifndef TIVIDY_WORK_ITEM_H
#define TIVIDY_WORK_ITEM_H

#include <string>
#include "WorkflowComponent.h"

/**
 * @brief A single, leaf work item in a workflow.
 *
 * WorkItem records whether a task has been completed. TIVIDY coordinates the
 * task but does not perform the real-world work; completion is recorded
 * explicitly with complete().
 */
class WorkItem : public WorkflowComponent {
public:
    /** @brief Construct an unnamed, incomplete work item. */
    WorkItem();

    /**
     * @brief Construct an incomplete work item with a descriptive name.
     * @param itemName Task name.
     */
    explicit WorkItem(const std::string& itemName);

    /**
     * @brief Execute the component interface without completing the task.
     */
    virtual void execute();

    /**
     * @brief Report whether the work item has been completed.
     * @return true after complete() has been called; otherwise false.
     */
    virtual bool isComplete() const;

    /** @brief Mark this work item as complete. */
    void complete();

    /** @brief Return the name of this work item. */
    const std::string& getName() const;

private:
    std::string name;
    bool completed;
};

#endif // TIVIDY_WORK_ITEM_H