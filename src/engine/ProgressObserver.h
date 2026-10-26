#ifndef PROGRESSOBSERVER_H
#define PROGRESSOBSERVER_H

#include "WorkflowObserver.h"
#include "WorkflowInstance.h"

class ProgressObserver :public WorkflowObserver {


public:
	virtual void update(WorkflowInstance* instance)override;
};

#endif
