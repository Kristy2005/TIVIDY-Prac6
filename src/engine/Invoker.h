#ifndef INVOKER_H
#define INVOKER_H

#include "Command.h"


namespace tividy {
	class Invoker {

	private:
		Command* command;

	public:
		void setCommand(Command* command);

		void executeCommand();

		void undoCommand();
	};

}

#endif // INVOKER_H
