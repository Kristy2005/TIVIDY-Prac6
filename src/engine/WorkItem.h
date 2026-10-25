#ifndef WORKITEM_H
#define WORKITEM_H

#include "WorkflowComponent.h"
#include <string>

class WorkItem :public WorkflowComponent {

private:
	std::string name;
	bool completed;

public:
	virtual void execute()override;

	bool isComplete();

	void complete();
};

#endif
