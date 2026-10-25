#ifndef ESCALATIONHANDLER_H
#define ESCALATIONHANDLER_H

class EscalationHandler : Handler {


public:
	void handleRequest(WorkflowInstance* instance);
};

#endif
