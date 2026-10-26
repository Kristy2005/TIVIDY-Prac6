#ifndef PRIORITYDECORATOR_H
#define PRIORITYDECORATOR_H

#include "WorkItemDecorator.h"

class PriorityDecorator :public WorkItemDecorator {

public:
	int priority;

	int getPriority();

	virtual void execute()override;

	virtual bool isComplete()override;
};

#endif
