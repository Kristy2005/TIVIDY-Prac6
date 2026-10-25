#ifndef CLOSEWORKFLOWCOMMAND_H
#define CLOSEWORKFLOWCOMMAND_H

#include "Command.h"

class CloseWorkflowCommand :public Command {


public:
	void execute();

	void undo();
};

#endif
