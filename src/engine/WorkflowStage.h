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

    /**
     * @brief Swap one child for another without changing its position.
     *
     * Used by the builder to put a decorator in the place of the item it
     * wraps. The stage never deletes @p oldComponent; the caller keeps
     * responsibility for it (it is non-owning in definition trees).
     *
     * @param oldComponent Child to replace; must currently be in this node.
     * @param newComponent Replacement; must not be null, this node, or
     *        already a child.
     * @return true if the replacement happened; otherwise false.
     */
    bool replace(WorkflowComponent* oldComponent, WorkflowComponent* newComponent);

    /** @brief Execute each child component. */
    virtual void execute();

    /**
     * @brief Check whether all children have completed.
     * @return true if every child is complete; an empty stage is complete.
     */
    virtual bool isComplete() const;
    virtual void collectWorkItems(std::vector<WorkItem*>& items);

    /**
     * @brief Destroys the workflow stage and its owned child components.
     */
    ~WorkflowStage();

    /**
     * @brief Creates a deep copy of this workflow stage and all child components.
     */
    WorkflowComponent* clone() const override;


private:
    /** @brief Child component pointers in execution order; not owned. */
    std::vector<WorkflowComponent*> components;

    /**
     * @brief Indicates whether this stage owns its child components.
     */
    bool ownsComponents = false;
};

}

#endif // TIVIDY_WORKFLOW_STAGE_H