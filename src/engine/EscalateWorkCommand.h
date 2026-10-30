#ifndef ESCALATEWORKCOMMAND_H
#define ESCALATEWORKCOMMAND_H

#include "Command.h"


namespace tividy {
/**
 * @brief Command that sends an escalation request for a workflow instance.
 *
 * The receiver handles escalation through its WorkflowDefinition's configured
 * handler chain. In the current implementation this does not itself change the
 * workflow state.
 *
 */
	class EscalateWorkCommand :public Command {
	public:
	/**
     * @brief Save the previous state and send the escalation request.
     * @throws std::logic_error If the receiver is unset, escalation is not
     *         allowed in the current state, or no handler is configured.
     */
		virtual void execute()override;

	/**
     * @brief Restore the state captured by the most recent execute call.
     * @throws std::logic_error If the receiver or saved memento is missing.
     */
		virtual void undo()override;
	};

}

#endif // ESCALATEWORKCOMMAND_H
