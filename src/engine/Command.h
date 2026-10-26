#ifndef COMMAND_H
#define COMMAND_H

class WorkflowInstance;

class Command {

protected:
	WorkflowInstance* receiver;

public:
	virtual void execute() = 0;

	virtual void undo() = 0;
};

#endif
