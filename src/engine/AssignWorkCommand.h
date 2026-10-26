#ifndef ASSIGNWORKCOMMAND_H
#define ASSIGNWORKCOMMAND_H

#include "Command.h"


namespace tividy {
	class AssignWorkCommand : public Command{


	public:
		virtual void execute()override;

		virtual void undo()override;
	};
}

#endif // ASSIGNWORKCOMMAND_H
