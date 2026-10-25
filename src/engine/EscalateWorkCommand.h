#ifndef ESCALATEWORKCOMMAND_H
#define ESCALATEWORKCOMMAND_H

#include "Command.h"

class EscalateWorkCommand :public Command {


public:
	virtual void execute()override;

	virtual void undo()override;
};

#endif
