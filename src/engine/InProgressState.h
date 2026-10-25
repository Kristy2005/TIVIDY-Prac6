#ifndef INPROGRESSSTATE_H
#define INPROGRESSSTATE_H

#include "WorkflowState.h"
#include "WorkflowInstance.h"

class InProgressState :public WorkflowState {


public:
	void escalate(WorkflowInstance* instance);

	void resolve(WorkflowInstance* instance);

	void close(WorkflowInstance* instance);
};

#endif
