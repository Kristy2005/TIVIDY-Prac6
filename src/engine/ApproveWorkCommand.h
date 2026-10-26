#ifndef APPROVEWORKCOMMAND_H
#define APPROVEWORKCOMMAND_H

#include "Command.h"


namespace tividy {
	class ApproveWorkCommand :public Command {

	public:
		virtual void execute()override;

		virtual void undo()override;
	};

}

#endif // APPROVEWORKCOMMAND_H
