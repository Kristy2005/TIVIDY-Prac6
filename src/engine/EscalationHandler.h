#ifndef ESCALATIONHANDLER_H
#define ESCALATIONHANDLER_H

#include "Handler.h"


namespace tividy {
/**
 * @brief Middle handler that forwards escalation to the next handler.
 *
 * A null request is rejected. If a successor exists, the request is passed to
 * it; otherwise this handler throws because the chain has no final decision
 * handler configured.
 *
 */
	class EscalationHandler :public Handler {
		public:
	/**
     * @brief Validate and forward an escalation request.
     * @param instance Workflow instance being escalated; must not be null.
     * @throws std::invalid_argument If @p instance is null.
     * @throws std::logic_error If no successor is configured.
     */
		void handleRequest(WorkflowInstance* instance);
	};

}

#endif // ESCALATIONHANDLER_H
