#ifndef CANCELWORKCOMMAND_H
#define CANCELWORKCOMMAND_H

#include "Command.h"

class CancelWorkCommand :public Command {


public:
	virtual void execute()override;

	virtual void undo()override;
};

#endif
