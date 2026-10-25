#ifndef COMPLETEWORKCOMMAND_H
#define COMPLETEWORKCOMMAND_H

#include "Command.h"

class CompleteWorkCommand :public Command {


public:
	virtual void execute()override;

	virtual void undo()override;
};

#endif
