#ifndef APPROVALHANDLER_H
#define APPROVALHANDLER_H

#include "Handler.h"
#include "WorkflowInstance.h"


namespace tividy {
/**
 * @brief First handler in the configured escalation chain.
 *
 * This handler validates the workflow pointer and forwards the request to its
 * successor when one is configured. With no successor it returns without
 * taking further action.
 * 
 */
	class ApprovalHandler :public Handler{
	public:
	/**
     * @brief Validate and forward an escalation request.
     * @param instance Workflow instance being escalated; must not be null.
     * @throws std::invalid_argument If @p instance is null.
	 * 
     */
		void handleRequest(WorkflowInstance* instance);
	};

}

#endif // APPROVALHANDLER_H
