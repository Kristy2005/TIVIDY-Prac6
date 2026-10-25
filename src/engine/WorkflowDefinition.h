#ifndef WORKFLOWDEFINITION_H
#define WORKFLOWDEFINITION_H

class WorkflowDefinition {

private:
	string name;
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
