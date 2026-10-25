#ifndef CONCRETEWORKFLOWBUILDER_H
#define CONCRETEWORKFLOWBUILDER_H

#include "WorkflowBuilder.h"
#include "Role.h" // Include Role.h so the compiler knows what Role<T> is

class WorkflowDefinition;

class ConcreteWorkflowBuilder : public WorkflowBuilder {
public:
	WorkflowDefinition* definition;

	void addWorkItem(WorkItem* item);

	// 1. Make this a proper member template function
	template <typename T>
	void addRole(Role<T>* role) {
		// Because it's a template, define it right here in the header!
		// TODO - implement ConcreteWorkflowBuilder::addRole
		throw "Not yet implemented";
	}

	void addDependency(WorkItem* before, WorkItem* after);
	void setAssignmentStrategy(AssignmentStrategy* strategy);
	void setEscalationHandler(Handler* handler);
	void addPriority(WorkItem* item, int priority);
	WorkflowDefinition* getResult();
};

#endif // CONCRETEWORKFLOWBUILDER_H
