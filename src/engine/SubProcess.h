#ifndef TIVIDY_SUB_PROCESS_H
#define TIVIDY_SUB_PROCESS_H

#include <vector>
#include "WorkflowComponent.h"

/**
 * @brief Composite workflow node representing a nested sub-process.
 *
 * SubProcess implements the same interface as a WorkItem or WorkflowStage, so
 * it can be nested in another stage or sub-process. Child pointers are
 * non-owning; the caller remains responsible for the pointed-to objects.
 */

namespace tividy {
class SubProcess : public WorkflowComponent {
public:
    /**
     * @brief Add a child component to this sub-process.
     * @param component Component to add. Null, duplicate, and direct
     * self-references are ignored.
     */
    void add(WorkflowComponent* component);

    /**
     * @brief Remove a child component from this sub-process.
     * @param component Component to remove. Null or absent pointers are ignored.
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
     * @brief Check whether all child components have completed.
     * @return true if every child is complete; an empty sub-process is complete.
     */
    virtual bool isComplete() const;
    virtual void collectWorkItems(std::vector<WorkItem*>& items);

    /**
     * @brief Destroys the subprocess and its owned child components.
     */
    ~SubProcess();

    /**
     * @brief Creates a deep copy of this subprocess and all child components.
     */
    WorkflowComponent* clone() const override;

private:
    /** @brief Child component pointers in execution order; not owned. */
    std::vector<WorkflowComponent*> components;

    /**
    * @brief Indicates whether this subprocess owns its child components.
    */
    bool ownsComponents = false;
};

}

#endif // TIVIDY_SUB_PROCESS_H