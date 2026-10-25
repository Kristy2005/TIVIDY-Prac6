#ifndef HISTORYOBSERVER_H
#define HISTORYOBSERVER_H

#include "WorkflowObserver.h"
#include "WorkflowInstance.h"

class HistoryObserver :public WorkflowObserver {


public:
	void update(WorkflowInstance* instance);
};

#endif
