#ifndef WORKFLOWSTAGE_H
#define WORKFLOWSTAGE_H

class WorkflowStage : WorkflowComponent {

public:
	vector<WorkflowComponent*> components;

	void add(WorkflowComponent* component);

	void remove(WorkflowComponent* component);

	void execute();

	bool isComplete();
};

#endif
