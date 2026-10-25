#ifndef INPROGRESSSTATE_H
#define INPROGRESSSTATE_H

#include "WorkflowState.h"
#include "WorkflowInstance.h"

class InProgressState :public WorkflowState {


public:
	virtual void activate(WorkflowInstance* instance)override;

	virtual void startProcessing(WorkflowInstance* instance)override;

	virtual void escalate(WorkflowInstance* instance)override;

	virtual void resolve(WorkflowInstance* instance)override;

	virtual void close(WorkflowInstance* instance)override;
};

#endif
