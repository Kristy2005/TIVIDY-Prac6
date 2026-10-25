#ifndef WORKHANDLER_H
#define WORKHANDLER_H

class WorkHandler : Handler {


public:
	void handleRequest(WorkflowInstance* instance);
};

#endif
