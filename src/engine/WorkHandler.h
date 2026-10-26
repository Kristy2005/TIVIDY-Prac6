#ifndef WORKHANDLER_H
#define WORKHANDLER_H

#include "Handler.h"
#include "WorkflowInstance.h"

namespace tividy {
	class WorkHandler :public tividy::Handler{ 


	public:
		void handleRequest(WorkflowInstance* instance);
	};

}

#endif // WORKHANDLER_H
