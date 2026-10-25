#ifndef CLOSEDSTATE_H
#define CLOSEDSTATE_H

#include "WorkflowState.h"

class ClosedState : public WorkflowState {
    virtual void activate(WorkflowInstance* instance)override;

	virtual void startProcessing(WorkflowInstance* instance)override;

	virtual void escalate(WorkflowInstance* instance)override;

	virtual void resolve(WorkflowInstance* instance)override;

	virtual void close(WorkflowInstance* instance)override;
};

#endif
