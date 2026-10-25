#ifndef CANCELWORKCOMMAND_H
#define CANCELWORKCOMMAND_H

class CancelWorkCommand : Command {


public:
	void execute();

	void undo();
};

#endif
