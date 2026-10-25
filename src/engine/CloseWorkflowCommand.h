#ifndef CLOSEWORKFLOWCOMMAND_H
#define CLOSEWORKFLOWCOMMAND_H

#include "Command.h"

class CloseWorkflowCommand :public Command {


public:
	virtual void execute()override;

	virtual void undo()override;
};

#endif
