#include "WorkflowDefinition.h"

namespace tividy {

void WorkflowDefinition::setRoot(WorkflowComponent* component) {
	this->root = component;
}

void WorkflowDefinition::setAssignmentStrategy(AssignmentStrategy* strategy) {
	// TODO - implement WorkflowDefinition::setAssignmentStrategy
	throw "Not yet implemented";
}

void WorkflowDefinition::setEscalationHandler(Handler* handler) {
	this->escalationHandler = handler;
}

WorkflowInstance* WorkflowDefinition::createInstance() {
	// TODO - implement WorkflowDefinition::createInstance
	throw "Not yet implemented";
}
}
