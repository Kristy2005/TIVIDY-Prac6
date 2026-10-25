#ifndef WORKLOADBASEDASSIGNMENT_H
#define WORKLOADBASEDASSIGNMENT_H

class WorkloadBasedAssignment : AssignmentStrategy {


public:
	Participant* assign(WorkItem* item, vector<Participant*> participants);
};

#endif
