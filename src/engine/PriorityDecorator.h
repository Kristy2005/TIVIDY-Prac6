#ifndef TIVIDY_PRIORITY_DECORATOR_H
#define TIVIDY_PRIORITY_DECORATOR_H

#include "WorkItemDecorator.h"

/**
 * @brief Decorator that marks the priority of a workflow component.
 *
 * ConcreteWorkflowBuilder::addPriority() wraps an item in this decorator and
 * also stores the same value on the item itself, so the priority survives
 * cloning into a workflow instance and is visible to assignment strategies
 * through WorkItem::getPriority(). The decorator does not change the wrapped
 * item's completion state.
 */

namespace tividy {
    class PriorityDecorator : public WorkItemDecorator {
    public:
        /**
         * @brief Decorate a component with a priority value.
         * @param component Component to wrap; must not be null.
         * @param priority Priority metadata to expose to routing/assignment logic.
         * @throws std::invalid_argument when component is null.
         */
        PriorityDecorator(WorkflowComponent* component, int priority);

        /** @brief Return this decorator's priority value. */
        int getPriority() const;

        /**
         * @brief Change this decorator's priority value.
         * @param priority New non-negative priority.
         * @throws std::invalid_argument If @p priority is negative.
         */
        void setPriority(int priority);

        /** @brief Execute the wrapped component */
        virtual void execute();

        /** @brief Return the wrapped component's completion status */
        virtual bool isComplete() const;

        /**
         * @brief Creates an independent copy of this decorator and its wrapped component.
         *
         * The copy owns the cloned wrapped component and deletes it.
         */
        WorkflowComponent* clone() const override;

    private:
        /** @brief Priority data stored by this decorator. */
        int priority;
    };

}

#endif // TIVIDY_PRIORITY_DECORATOR_H