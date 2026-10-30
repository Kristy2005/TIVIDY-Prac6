#ifndef ASSIGNWORKCOMMAND_H
#define ASSIGNWORKCOMMAND_H

#include "Command.h"


namespace tividy {
/**
 * @brief Command that starts processing on a workflow instance.
 *
 * Despite the class name, this command does not select a Participant or assign
 * a WorkItem. It saves the workflow state and calls
 * WorkflowInstance::startProcessing().
 *
 */
	class AssignWorkCommand : public Command{


	public:
	/**
     * @brief Save the previous state and start processing.
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

#endif // ASSIGNWORKCOMMAND_H
