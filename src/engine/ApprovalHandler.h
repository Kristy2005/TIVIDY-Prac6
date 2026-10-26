#ifndef APPROVALHANDLER_H
#define APPROVALHANDLER_H

#include "Handler.h"
#include "WorkflowInstance.h"


namespace tividy {
class ApprovalHandler :public Handler{

public:
	void handleRequest(WorkflowInstance* instance);
};

#endif
}

