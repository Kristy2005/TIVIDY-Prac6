#ifndef WORKFLOWHISTORY_H
#define WORKFLOWHISTORY_H

#include "WorkflowMemento.h"

#include <vector>

class WorkflowHistory {

private:
	std::vector<WorkflowMemento*> mementos;

public:
	void save(WorkflowMemento* memento);

	WorkflowMemento* getLast();
};

#endif
