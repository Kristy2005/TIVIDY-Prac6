#ifndef HANDLER_H
#define HANDLER_H

#include "WorkflowInstance.h"

class Handler {

private:
	Handler* successor;

public:
	void setSuccessor(Handler* handler);

	void handleRequest(WorkflowInstance* instance);
};

#endif
