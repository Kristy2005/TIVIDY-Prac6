#ifndef NOTIFICATIONOBSERVER_H
#define NOTIFICATIONOBSERVER_H

#include "WorkflowObserver.h"

namespace tividy {

/**
 * @brief Observer pattern: ConcreteObserver. Intended to tell a participant
 *        when an instance changes.
 *
 * It is meant to send through the NotificationService interface only, so it
 * never knows whether the message goes out by email or any other channel (see
 * EmailAdapter).
 *
 * @warning Notification delivery is not implemented yet. update() only checks
 *          that the instance is not null. The class has no recipient and no
 *          NotificationService, so nothing is sent.
 *
 * @see WorkflowObserver, NotificationService, EmailAdapter
 */
class NotificationObserver : public WorkflowObserver {

public:
	/**
	 * @brief Checks the instance. Does not send anything yet.
	 *
	 * @param instance The instance that changed. Not stored.
	 */
	virtual void update(WorkflowInstance* instance) override;
};

} // namespace tividy

#endif // NOTIFICATIONOBSERVER_H