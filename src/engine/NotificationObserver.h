#ifndef NOTIFICATIONOBSERVER_H
#define NOTIFICATIONOBSERVER_H

#include "WorkflowObserver.h"

class NotificationObserver :public WorkflowObserver {


public:
	virtual void update(WorkflowInstance* instance)override;
};

#endif
