#ifndef HANDLER_H
#define HANDLER_H

// #include "WorkflowInstance.h"

namespace tividy {
class WorkflowInstance;

class Handler {

private:
	Handler* successor;

public:
//to be overridden later 
	virtual void setSuccessor(Handler* handler);

	virtual void handleRequest(WorkflowInstance* instance);
};

#endif
}

