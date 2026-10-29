#ifndef CANCELWORKCOMMAND_H
#define CANCELWORKCOMMAND_H

#include "Command.h"


namespace tividy {
/**
 * @brief Command that closes the receiver's workflow instance.
 *
 * The command records a memento before requesting closure. Its undo operation
 * restores the recorded state when both the receiver and memento are present.
 *
 */
	class CancelWorkCommand :public Command {


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

#endif // CANCELWORKCOMMAND_H
