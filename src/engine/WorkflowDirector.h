#ifndef WORKFLOWDIRECTOR_H
#define WORKFLOWDIRECTOR_H

class WorkflowDirector {

private:
	WorkflowBuilder* builder;

public:
	void setBuilder(WorkflowBuilder* builder);

	void constructWorkflow();
};

#endif
