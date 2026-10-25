#ifndef INVOKER_H
#define INVOKER_H

class Invoker {

private:
	Command* command;

public:
	void setCommand(Command* command);

	void executeCommand();

	void undoCommand();
};

#endif
