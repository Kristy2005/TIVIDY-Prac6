#ifndef INVOKER_H
#define INVOKER_H

#include "Command.h"


namespace tividy {
/**
 * @brief Invokes and undoes a configured Command.
 *
 * The invoker stores a non-owning command pointer. The command must remain
 * alive while it is configured; the invoker does not delete it.
 *
 */
	class Invoker {
	private:
	    /** @brief Command currently selected for execution; not owned. */
		Command* command;

	public:
    /** @brief Construct an invoker with no command configured. */
        Invoker();

	/**
     * @brief Select the command to execute or undo.
     * @param command Command to retain without taking ownership; may be null.
     */
		void setCommand(Command* command);

	/**
     * @brief Execute the configured command.
     * @throws std::logic_error If no command is configured.
     */
		void executeCommand();

	/**
     * @brief Undo the configured command.
     * @throws std::logic_error If no command is configured.
     */
		void undoCommand();
	};

}

#endif // INVOKER_H
