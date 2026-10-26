#ifndef TIVIDY_WORKFLOW_COMPONENT_H
#define TIVIDY_WORKFLOW_COMPONENT_H

/**
 * @brief Common interface for a work item or a composite workflow node.
 *
 * WorkflowComponent is the Component participant in the Composite pattern.
 * Clients can execute or query a WorkItem, WorkflowStage, SubProcess, or a
 * decorator without needing to know the concrete type.
 */
class WorkflowComponent {
public:
    /** @brief Virtual destructor. */
    virtual ~WorkflowComponent() {}

    /**
     * @brief Execute this workflow component.
     *
     * Composite components delegate execution to their children. The engine
     * coordinates work rather than performing the real-world task represented
     * by a WorkItem, so WorkItem execution does not itself mark the item complete.
     */
    virtual void execute() = 0;

    /**
     * @brief Check whether this component has completed its work.
     * @return true when the component is complete; otherwise false.
     */
    virtual bool isComplete() const = 0;
};

#endif // TIVIDY_WORKFLOW_COMPONENT_H
