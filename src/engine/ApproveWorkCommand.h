#ifndef APPROVEWORKCOMMAND_H
#define APPROVEWORKCOMMAND_H

#include "Command.h"

namespace tividy {
/**
 * @brief Command that activates a submitted workflow instance.
 *
 * The command saves a memento before calling WorkflowInstance::activate().
 * Undo asks the receiver to restore that saved state; it does not undo any
 * external side effects performed by observers.
 *
 */
	class ApproveWorkCommand :public Command {

	public:
	/**
     * @brief Save the previous state and activate the receiver.
     * @throws std::logic_error If no receiver is configured or activation is
     *         not permitted by the current state.
     */
		virtual void execute()override;

	/**
     * @brief Restore the state captured by the most recent execute call.
     * @throws std::logic_error If the receiver or saved memento is missing.
     */
		virtual void undo()override;
	};

}

#endif // APPROVEWORKCOMMAND_H
