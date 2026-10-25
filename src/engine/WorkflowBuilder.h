#ifndef WORKFLOWBUILDER_H
#define WORKFLOWBUILDER_H

class WorkflowBuilder {


public:
	virtual void addWorkItem(WorkItem* item) = 0;

	virtual void addRole(Role* role) = 0;

	virtual void addDependency(WorkItem* before, WorkItem* after) = 0;

	void setAssignmentStrategy(AssignmentStrategy* strategy);

	virtual void setEscalationHandler(Handler* handler) = 0;

	virtual void addPriority(WorkItem* item, int priority) = 0;

	virtual WorkflowDefinition* getResult() = 0;
};

#endif
