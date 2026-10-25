#ifndef WORKFLOWSTATE_H
#define WORKFLOWSTATE_H

class WorkflowState {


public:
	virtual void activate(WorkflowInstance* instance) = 0;

	virtual void startProcessing(WorkflowInstance* instance) = 0;

	virtual void escalate(WorkflowInstance* instance) = 0;

	virtual void resolve(WorkflowInstance* instance) = 0;

	virtual void close(WorkflowInstance* instance) = 0;
};

#endif
