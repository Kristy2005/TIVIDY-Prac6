#ifndef PARTICIPANT_H
#define PARTICIPANT_H

class Participant {

private:
	string name;
	int workload;

public:
	void assignWork(WorkItem* item);

	void completeWork(WorkItem* item);

	int getWorkload();
};

#endif
