#ifndef WORKFLOWHISTORY_H
#define WORKFLOWHISTORY_H

class WorkflowHistory {

private:
	vector<WorkflowMemento*> mementos;

public:
	void save(WorkflowMemento* memento);

	WorkflowMemento* getLast();
};

#endif
