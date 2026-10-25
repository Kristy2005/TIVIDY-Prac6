#ifndef SUBMITTEDSTATE_H
#define SUBMITTEDSTATE_H

#include "WorkflowState.h"

class SubmittedState : WorkflowState {


public:
	void activate(WorkflowInstance* instance);
};

#endif
