#ifndef WORKHANDLER_H
#define WORKHANDLER_H

#include "Handler.h"

class WorkHandler :public Handler {


public:
	void handleRequest(WorkflowInstance* instance);
};

#endif
