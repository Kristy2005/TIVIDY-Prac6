#ifndef ASSIGNMENTSTRATEGY_H
#define ASSIGNMENTSTRATEGY_H

#include <vector>

#include "Participant.h"

namespace tividy {
class WorkItem;
class Participant;

class AssignmentStrategy {


public:
	virtual Participant* assign(WorkItem* item, std::vector<Participant*> participants) = 0;
};

#endif
}

