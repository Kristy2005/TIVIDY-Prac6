#ifndef WORKFLOWDEFINITION_H
#define WORKFLOWDEFINITION_H

#include "WorkflowComponent.h"
#include "AssignmentStrategy.h"
#include "Handler.h"
#include <string>


namespace tividy {
class WorkflowDefinition {

private:
	std::string name;
	WorkflowComponent* root;
	AssignmentStrategy* strategy;
	Handler* escalationHandler;

public:
	void setRoot(WorkflowComponent* component);

	void setAssignmentStrategy(AssignmentStrategy* strategy);

	void setEscalationHandler(Handler* handler);

	WorkflowInstance* createInstance();
};

#endif
}

