#ifndef ASSIGNMENTSTRATEGY_H
#define ASSIGNMENTSTRATEGY_H

#include "Participant.h"

class AssignmentStrategy {


public:
	virtual Participant* assign(WorkItem* item, vector<Participant*> participants) = 0;
};

#endif
