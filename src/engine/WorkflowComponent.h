#ifndef WORKFLOWCOMPONENT_H
#define WORKFLOWCOMPONENT_H

class WorkflowComponent {


public:
	virtual void execute() = 0;

	virtual bool isComplete() = 0;
};

#endif
