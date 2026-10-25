#ifndef ACTIVESTATE_H
#define ACTIVESTATE_H

class ActiveState : WorkflowState {


public:
	void startProcessing(WorkflowInstance* instance);

	void close(WorkflowInstance* instance);
};

#endif
