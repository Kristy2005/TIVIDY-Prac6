#ifndef WORKITEMDECORATOR_H
#define WORKITEMDECORATOR_H

#include "WorkflowComponent.h"

class WorkItemDecorator :public WorkflowComponent {

public:
	WorkflowComponent* component;

	virtual void execute() = 0;

	virtual bool isComplete() = 0;
};

#endif
