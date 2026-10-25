#ifndef ASSIGNWORKCOMMAND_H
#define ASSIGNWORKCOMMAND_H

#include "Command.h"

class AssignWorkCommand : public Command{


public:
	virtual void execute()override;

	virtual void undo()override;
};

#endif
