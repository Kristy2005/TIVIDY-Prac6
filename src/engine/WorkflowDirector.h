#ifndef WORKFLOWDIRECTOR_H
#define WORKFLOWDIRECTOR_H

#include "WorkflowBuilder.h"

class WorkflowDirector {

private:
	WorkflowBuilder* builder;

public:
	void setBuilder(WorkflowBuilder* builder);

	void constructWorkflow();
};

#endif
