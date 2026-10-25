#ifndef WORKLOADBASEDASSIGNMENT_H
#define WORKLOADBASEDASSIGNMENT_H

#include "AssignmentStrategy.h"

class WorkloadBasedAssignment :public AssignmentStrategy {


public:
	Participant* assign(WorkItem* item, vector<Participant*> participants);
};

#endif
