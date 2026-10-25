#ifndef NOTIFICATIONOBSERVER_H
#define NOTIFICATIONOBSERVER_H

class NotificationObserver : WorkflowObserver {


public:
	void update(WorkflowInstance* instance);
};

#endif
