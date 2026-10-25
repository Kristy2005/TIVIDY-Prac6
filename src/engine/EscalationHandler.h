#ifndef ESCALATIONHANDLER_H
#define ESCALATIONHANDLER_H

#include "Handler.h"

class EscalationHandler :public Handler {


public:
	void handleRequest(WorkflowInstance* instance);
};

#endif
