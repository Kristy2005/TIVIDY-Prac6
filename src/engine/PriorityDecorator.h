#ifndef PRIORITYDECORATOR_H
#define PRIORITYDECORATOR_H

class PriorityDecorator : WorkItemDecorator {

public:
	int priority;

	int getPriority();

	void execute();

	bool isComplete();
};

#endif
