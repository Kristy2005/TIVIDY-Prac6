#ifndef CLOSEWORKFLOWCOMMAND_H
#define CLOSEWORKFLOWCOMMAND_H

class CloseWorkflowCommand : Command {


public:
	void execute();

	void undo();
};

#endif
