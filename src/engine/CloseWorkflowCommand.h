#ifndef CLOSEWORKFLOWCOMMAND_H
#define CLOSEWORKFLOWCOMMAND_H

#include "Command.h"


namespace tividy {
	class CloseWorkflowCommand :public Command {


	public:
		virtual void execute()override;

		virtual void undo()override;
	};

}

#endif // CLOSEWORKFLOWCOMMAND_H
