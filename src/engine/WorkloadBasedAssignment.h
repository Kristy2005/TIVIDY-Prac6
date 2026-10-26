#ifndef WORKLOADBASEDASSIGNMENT_H
#define WORKLOADBASEDASSIGNMENT_H

#include "AssignmentStrategy.h"


namespace tividy {
class WorkloadBasedAssignment :public AssignmentStrategy {


public:
	Participant* assign(WorkItem* item, std::vector<Participant*> participants);
};

#endif
}

