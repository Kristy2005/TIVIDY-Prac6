#ifndef COMPLETEWORKCOMMAND_H
#define COMPLETEWORKCOMMAND_H

#include "Command.h"


namespace tividy {
/**
 * @brief Command that requests resolution of a workflow instance.
 *
 * The current implementation does not complete an individual WorkItem; it
 * saves a workflow snapshot and calls WorkflowInstance::resolve().
 *
 */
	class CompleteWorkCommand :public Command {
	public:
	/**
     * @brief Save the previous state and request workflow resolution.
     * @throws std::logic_error If the receiver is unset or the state rejects
     *         resolution.
     */
		virtual void execute()override;

	/**
     * @brief Restore the state captured by the most recent execute call.
     * @throws std::logic_error If the receiver or saved memento is missing.
     */
		virtual void undo()override;
	};

}

#endif // COMPLETEWORKCOMMAND_H
