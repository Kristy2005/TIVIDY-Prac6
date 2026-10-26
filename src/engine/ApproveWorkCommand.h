#ifndef APPROVEWORKCOMMAND_H
#define APPROVEWORKCOMMAND_H

#include "Command.h"

class ApproveWorkCommand :public Command {

public:
	virtual void execute()override;

	virtual void undo()override;
};

#endif
