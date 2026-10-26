#ifndef PARTICIPANT_H
#define PARTICIPANT_H

#include <string>
#include "WorkItem.h"


namespace tividy {
class Participant {

private:
	std::string name;
	int workload;

public:
	void assignWork(WorkItem* item);

	void completeWork(WorkItem* item);

	int getWorkload();
};

#endif
}

