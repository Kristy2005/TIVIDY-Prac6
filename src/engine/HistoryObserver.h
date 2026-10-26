#ifndef HISTORYOBSERVER_H
#define HISTORYOBSERVER_H

#include "WorkflowObserver.h"
// #include "WorkflowInstance.h"

class HistoryObserver :public WorkflowObserver {


public:
	virtual void update(WorkflowInstance* instance)override;
};

#endif
