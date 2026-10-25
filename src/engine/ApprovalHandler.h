#ifndef APPROVALHANDLER_H
#define APPROVALHANDLER_H

#include "Handler.h"
#include "WorkflowInstance.h"

class ApprovalHandler :public Handler{

public:
	virtual void handleRequest(WorkflowInstance* instance)override;
};

#endif
