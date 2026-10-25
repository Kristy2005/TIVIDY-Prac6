#ifndef ACTIVESTATE_H
#define ACTIVESTATE_H

#include "WorkflowState.h"
#include "WorkflowInstance.h"

class ActiveState :public WorkflowState {

public:
	virtual void startProcessing(WorkflowInstance* instance)override;

	virtual void close(WorkflowInstance* instance)override;
};

#endif
