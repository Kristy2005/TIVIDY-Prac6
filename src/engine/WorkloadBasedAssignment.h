#ifndef WORKLOADBASEDASSIGNMENT_H
#define WORKLOADBASEDASSIGNMENT_H

#include "AssignmentStrategy.h"


namespace tividy {
	class WorkloadBasedAssignment : public tividy::AssignmentStrategy {


	public:
		Participant* assign(WorkItem* item, std::vector<Participant*> participants);
	};

	#endif // WORKLOADBASEDASSIGNMENT_H
}

