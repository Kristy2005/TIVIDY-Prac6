#ifndef WORKITEMDECORATOR_H
#define WORKITEMDECORATOR_H

class WorkItemDecorator : WorkflowComponent {

public:
	WorkflowComponent* component;

	virtual void execute() = 0;

	virtual bool isComplete() = 0;
};

#endif
