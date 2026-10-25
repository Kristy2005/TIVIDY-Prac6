#ifndef HISTORYOBSERVER_H
#define HISTORYOBSERVER_H

class HistoryObserver : WorkflowObserver {


public:
	void update(WorkflowInstance* instance);
};

#endif
