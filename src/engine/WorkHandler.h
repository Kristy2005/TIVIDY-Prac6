#ifndef WORKHANDLER_H
#define WORKHANDLER_H

#include "Handler.h"
#include "WorkflowInstance.h"

/*
*@brief 
*/

namespace tividy {
	class WorkHandler :public tividy::Handler{ 


	public:
	/*
	*@brief Handle a workflow request.
	*@param instance The workflow instance to handle. Must not be null.
	*@throws std::invalid_argument If instance is null.
	*/
		void handleRequest(WorkflowInstance* instance);
	};

}

#endif // WORKHANDLER_H
