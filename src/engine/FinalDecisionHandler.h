#ifndef FINALDECISIONHANDLER_H
#define FINALDECISIONHANDLER_H

#include "Handler.h"

class FinalDecisionHandler :public Handler {


public:
	void handleRequest(WorkflowInstance* instance);
};

#endif
