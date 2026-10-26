#ifndef WORKFLOWSTAGE_H
#define WORKFLOWSTAGE_H

#include "WorkflowComponent.h"
#include <vector>

class WorkflowStage :public WorkflowComponent {

public:
	std::vector<WorkflowComponent*> components;

	void add(WorkflowComponent* component);

	void remove(WorkflowComponent* component);

	virtual void execute()override;

	virtual bool isComplete()override;
};

#endif
