#ifndef NOTIFICATIONOBSERVER_H
#define NOTIFICATIONOBSERVER_H

#include "WorkflowObserver.h"
#include "WorkflowInstance.h"

class NotificationObserver :public WorkflowObserver {


public:
	void update(WorkflowInstance* instance);
};

#endif
