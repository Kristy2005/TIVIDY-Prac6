#ifndef CLOSEWORKFLOWCOMMAND_H
#define CLOSEWORKFLOWCOMMAND_H

#include "Command.h"


namespace tividy {
/**
 * @brief Command that closes a workflow instance.
 *
 * It is functionally equivalent to CancelWorkCommand in the current
 * implementation: execution saves a state snapshot and calls close().
 *
 */
	class CloseWorkflowCommand :public Command {


	public:
    /**
     * @brief Save the previous state and close the receiver.
     * @throws std::logic_error If no receiver is set or the transition is
     *         rejected by the current state.
     */
		virtual void execute()override;

    /**
     * @brief Restore the state captured by the most recent execute call.
     * @throws std::logic_error If the receiver or saved memento is missing.
     */
		virtual void undo()override;
	};

}

#endif // CLOSEWORKFLOWCOMMAND_H
