#ifndef PROGRESSOBSERVER_H
#define PROGRESSOBSERVER_H

class ProgressObserver : WorkflowObserver {


public:
	void update(WorkflowInstance* instance);
};

#endif
