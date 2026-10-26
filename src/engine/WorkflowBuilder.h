#ifndef TIVIDY_WORKFLOW_BUILDER_H
#define TIVIDY_WORKFLOW_BUILDER_H

#include "WorkItem.h"
#include "Role.h"
#include "AssignmentStrategy.h"
#include "Handler.h"

class WorkflowDefinition;

/**
 * @brief Interface for step-by-step construction of workflow definitions.
 */
class WorkflowBuilder {
public:
    /** @brief Virtual destructor for polymorphic builder use. */
    virtual ~WorkflowBuilder() {}

    /** @brief Add a work item to the workflow being built. */
    virtual void addWorkItem(WorkItem* item) = 0;

    /** @brief Add a role to the workflow being built. */
    virtual void addRole(Role<void>* role) = 0;

    /** @brief Add a dependency between two work items. */
    virtual void addDependency(WorkItem* before, WorkItem* after) = 0;

    /** @brief Select the assignment algorithm for the definition being built. */
    virtual void setAssignmentStrategy(AssignmentStrategy* strategy) = 0;

    /** @brief Configure escalation handling for the definition being built. */
    virtual void setEscalationHandler(Handler* handler) = 0;

    /** @brief Decorate a work item with priority during workflow construction. */
    virtual void addPriority(WorkItem* item, int priority) = 0;

    /** @brief Return the finished definition and transfer ownership to the caller. */
    virtual WorkflowDefinition* getResult() = 0;
};

#endif // TIVIDY_WORKFLOW_BUILDER_H
