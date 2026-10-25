#ifndef SUBPROCESS_H
#define SUBPROCESS_H

#include "WorkflowComponent.h"
#include <vector>

class SubProcess :public WorkflowComponent {

public:
	std::vector<WorkflowComponent*> components;

	void add(WorkflowComponent* component);

	void remove(WorkflowComponent* component);

	void execute();

	bool isComplete();
};

#endif
