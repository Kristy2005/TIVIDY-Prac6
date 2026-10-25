#ifndef ACTIVESTATE_H
#define ACTIVESTATE_H

#include "WorkflowState.h"
class WorkflowInstance;

class ActiveState :public WorkflowState {

public:
	void startProcessing(WorkflowInstance* instance);

	void close(WorkflowInstance* instance);
};

#endif
