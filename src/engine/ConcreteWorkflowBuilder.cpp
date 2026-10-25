#include "ConcreteWorkflowBuilder.h"

void ConcreteWorkflowBuilder::addWorkItem(WorkItem* item) {
	throw "Not yet implemented";
}

// REMOVED addRole from here entirely!

void ConcreteWorkflowBuilder::addDependency(WorkItem* before, WorkItem* after) {
	throw "Not yet implemented";
}

void ConcreteWorkflowBuilder::setAssignmentStrategy(AssignmentStrategy* strategy) {
	throw "Not yet implemented";
}

void ConcreteWorkflowBuilder::setEscalationHandler(Handler* handler) {
	throw "Not yet implemented";
}

void ConcreteWorkflowBuilder::addPriority(WorkItem* item, int priority) {
	throw "Not yet implemented";
}

WorkflowDefinition* ConcreteWorkflowBuilder::getResult() {
	throw "Not yet implemented";
}
