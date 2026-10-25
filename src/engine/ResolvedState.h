#ifndef RESOLVEDSTATE_H
#define RESOLVEDSTATE_H

#include "WorkflowState.h"

class ResolvedState :public WorkflowState {
    virtual void activate(WorkflowInstance* instance)override;

	virtual void startProcessing(WorkflowInstance* instance)override;

	virtual void escalate(WorkflowInstance* instance)override;

	virtual void resolve(WorkflowInstance* instance)override;

	virtual void close(WorkflowInstance* instance)override;
};

#endif //RESOLVEDSTATE_H