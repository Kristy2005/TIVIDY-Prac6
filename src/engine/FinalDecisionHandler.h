#ifndef FINALDECISIONHANDLER_H
#define FINALDECISIONHANDLER_H

#include "Handler.h"


namespace tividy {
	class FinalDecisionHandler :public Handler {


	public:
		void handleRequest(WorkflowInstance* instance);
	};

}

#endif // FINALDECISIONHANDLER_H
