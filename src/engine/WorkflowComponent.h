#ifndef TIVIDY_WORKFLOW_COMPONENT_H
#define TIVIDY_WORKFLOW_COMPONENT_H

#include <vector>

namespace tividy {

class WorkItem;

/**
 * @brief Common interface for a work item or composite workflow node.
 *
 * WorkflowComponent is the Component participant in the Composite pattern.
 */

class WorkflowComponent {
public:
    /**
     * @brief Creates an independent copy of this workflow component.
     */
    virtual WorkflowComponent* clone() const = 0;

    virtual ~WorkflowComponent() {}

    /**
     * @brief Execute this workflow component.
     */
    virtual void execute() = 0;

    /**
     * @brief Check whether this component has completed its work.
     */
    virtual bool isComplete() const = 0;

    /**
     * @brief Collect all work items contained in this component.
     * @param items Collection to populate with work item pointers.
     */
    virtual void collectWorkItems(std::vector<WorkItem*>& items) {
    (void)items;
}
};

}

#endif
