#ifndef WORKFLOWOBSERVER_H
#define WORKFLOWOBSERVER_H

#include "WorkflowInstance.h"
class WorkflowInstance;

class WorkflowObserver {

public:
/**
 * @brief: 
 */
	virtual void update(WorkflowInstance* instance) = 0;
};

#endif
