#ifndef ROLEBASEDASSIGNMENT_H
#define ROLEBASEDASSIGNMENT_H

class RoleBasedAssignment : AssignmentStrategy {


public:
	Participant* assign(WorkItem* item, vector<Participant*> participants);
};

#endif
