#ifndef ROLEBASEDASSIGNMENT_H
#define ROLEBASEDASSIGNMENT_H

#include "AssignmentStrategy.h"
#include <vector>


namespace tividy {
class RoleBasedAssignment :public AssignmentStrategy {


public:
	Participant* assign(WorkItem* item, std::vector<Participant*> participants);
};

#endif
}

