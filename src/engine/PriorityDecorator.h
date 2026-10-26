#ifndef TIVIDY_PRIORITY_DECORATOR_H
#define TIVIDY_PRIORITY_DECORATOR_H

#include "WorkItemDecorator.h"

/**
 * @brief Decorator that adds marks the priority of a workflow component
 * 
 * The assignment strategy can read getPriority() when deciding which
 * item to assign. This decorator does not own the wrapper component and does
 * not change the wrapper item's completion state itself.
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

        /** Execute the wrapped component */
        virtual void execute();

        /** Return the wrapped components completion status */
        virtual bool isComplete() const;

    private:
        int priority;
    };

}

#endif // TIVIDY_PRIORITY_DECORATOR_H
