#ifndef TIVIDY_WORK_ITEM_DECORATOR_H
#define TIVIDY_WORK_ITEM_DECORATOR_H

#include "WorkflowComponent.h"

/**
 * @brief Abstract decorator base for adding behaviour to a workflow component.
 *
 * The decorator holds a non-owning pointer to the wrapped component. Concrete
 * decorators must implement execute() and isComplete(), normally delegating
 * to the wrapped component while adding their own behaviour.
 */

namespace tividy {
class WorkItemDecorator : public WorkflowComponent {
public:
    /**
     * @brief Construct a decorator around an existing component.
     * @param wrapped Component to decorate; must not be null.
     * @throws std::invalid_argument when wrapped is null.
     */
    explicit WorkItemDecorator(WorkflowComponent* wrapped);

    /** @brief Inherited virtual destruction is preserved through the base. */
    virtual ~WorkItemDecorator();

    /** @brief Execute the decorated component; implemented by subclasses. */
    virtual void execute() = 0;

    /**
     * @brief Ask about completion of the decorated component; implemented by subclasses.
     * @return Whether the decorated component is complete.
     */
    virtual bool isComplete() const = 0;
    virtual void collectWorkItems(std::vector<WorkItem*>& items);

protected:
    /** Non-owning pointer to the component wrapped by this decorator. */
    WorkflowComponent* component;
};

#endif // TIVIDY_WORK_ITEM_DECORATOR_H
}

