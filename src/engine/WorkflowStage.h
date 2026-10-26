#ifndef TIVIDY_WORKFLOW_STAGE_H
#define TIVIDY_WORKFLOW_STAGE_H

#include <vector>
#include "WorkflowComponent.h"

/**
 * @brief Composite workflow node that groups components into a stage.
 *
 * The stage stores non-owning pointers. The caller that creates a component
 * remains responsible for its lifetime and must keep it alive while this stage
 * refers to it. Children are executed in the order in which they were added.
 */

namespace tividy {
class WorkflowStage : public WorkflowComponent {
public:
    /**
     * @brief Add a child component to the stage.
     * @param component Component to add.
     */
    void add(WorkflowComponent* component);

    /**
     * @brief Remove a child component from the stage.
     * @param component Component to remove. Null pointers are ignored.
     */
    void remove(WorkflowComponent* component);

    /** @brief Execute each child component. */
    virtual void execute();

    /**
     * @brief Check whether all children have completed.
     * @return true if every child is complete; an empty stage is complete.
     */
    virtual bool isComplete() const;

private:
    std::vector<WorkflowComponent*> components;
};

#endif // TIVIDY_WORKFLOW_STAGE_H
}

