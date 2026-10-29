#ifndef FINALDECISIONHANDLER_H
#define FINALDECISIONHANDLER_H

#include "Handler.h"


namespace tividy {
/**
 * @brief Terminal handler for an escalation chain.
 *
 * It validates the supplied workflow instance and accepts the request, but
 * currently records no decision and does not change the workflow state.
 *
 */
	class FinalDecisionHandler :public Handler {
	public:
	/**
     * @brief Accept an escalation request at the end of the chain.
     * @param instance Workflow instance being escalated; must not be null.
     * @throws std::invalid_argument If @p instance is null.
     */
		void handleRequest(WorkflowInstance* instance);
	};

}

#endif // FINALDECISIONHANDLER_H
