#ifndef PROGRESSOBSERVER_H
#define PROGRESSOBSERVER_H

#include "WorkflowObserver.h"
#include "WorkflowInstance.h"

class ProgressObserver :public WorkflowObserver {


public:
	void update(WorkflowInstance* instance);
};

#endif
