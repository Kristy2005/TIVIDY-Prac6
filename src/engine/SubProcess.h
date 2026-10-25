#ifndef SUBPROCESS_H
#define SUBPROCESS_H

class SubProcess : WorkflowComponent {

public:
	vector<WorkflowComponent*> components;

	void add(WorkflowComponent* component);

	void remove(WorkflowComponent* component);

	void execute();

	bool isComplete();
};

#endif
