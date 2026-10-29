#ifndef WORKHANDLER_H
#define WORKHANDLER_H

#include "Handler.h"
#include "WorkflowInstance.h"

/**
 * @brief Handler that validates a workflow request and forwards it.
 *
 * If no successor is configured, the valid request is accepted with no further
 * action.
 *
 */

namespace tividy {
	class WorkHandler :public tividy::Handler{ 
	public:
	/**
     * @brief Validate and forward a workflow request.
     * @param instance Workflow instance to handle; must not be null.
     * @throws std::invalid_argument If @p instance is null.
     */
		void handleRequest(WorkflowInstance* instance);
	};

}

#endif // WORKHANDLER_H
