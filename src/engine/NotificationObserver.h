#ifndef NOTIFICATIONOBSERVER_H
#define NOTIFICATIONOBSERVER_H

#include "WorkflowObserver.h"

/**
 * @brief Observer pattern: ConcreteObserver. Tells a participant when an
 *        instance changes.
 *
 * Sends through the NotificationService interface only, so it never knows
 * whether the message goes out by email or any other channel (see
 * EmailAdapter).
 *
 * @par Ownership
 * Holds the recipient and the service but owns neither. The creator
 * deletes them, and must outlive this observer.
 *
 * @see WorkflowObserver, NotificationService, EmailAdapter
 */


namespace tividy {
	class NotificationObserver :public WorkflowObserver {


	public:
		virtual void update(WorkflowInstance* instance)override;
	};

}

#endif // NOTIFICATIONOBSERVER_H
