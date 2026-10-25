#ifndef WORKITEM_H
#define WORKITEM_H

class WorkItem : WorkflowComponent {

private:
	string name;
	bool completed;

public:
	void execute();

	bool isComplete();

	void complete();
};

#endif
