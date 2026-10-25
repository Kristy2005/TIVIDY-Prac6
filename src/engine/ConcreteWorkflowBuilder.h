#ifndef CONCRETEWORKFLOWBUILDER_H
#define CONCRETEWORKFLOWBUILDER_H

#include "WorkflowBuilder.h"
// #include "WorkflowDefinition.h"
class WorkflowDefinition;

class ConcreteWorkflowBuilder :public WorkflowBuilder {

public:
	WorkflowDefinition* definition;

	void addWorkItem(WorkItem* item);

	void addRole(Role* role);

	void addDependency(WorkItem* before, WorkItem* after);

	void setAssignmentStrategy(AssignmentStrategy* strategy);

	void setEscalationHandler(Handler* handler);

	void addPriority(WorkItem* item, int priority);

	WorkflowDefinition* getResult();
};

#endif
