#ifndef WORKFLOWMEMENTO_H
#define WORKFLOWMEMENTO_H

// #include "WorkflowState.h"

namespace tividy {
class WorkflowState;

class WorkflowMemento {

private:
	WorkflowState* state;

public:
	WorkflowMemento();
	void setState(WorkflowState* state);
	WorkflowState* getState() const;
};

#endif

// WORKFLOWMEMENTO_H
}

