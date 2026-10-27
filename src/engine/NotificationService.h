#ifndef NOTIFICATIONSERVICE_H
#define NOTIFICATIONSERVICE_H

#include "Participant.h"
#include <string>

namespace tividy {

/**
 * @brief Adapter pattern: Target. The interface TIVIDY uses to send a
 *        message to a participant.
 *
 * The engine calls only this interface, so it never depends on how a message
 * is delivered. A new channel is a new subclass, with no change to the
 * engine.
 *
 * @see EmailAdapter, NotificationObserver
 */
class NotificationService {

public:
	/**
	 * @brief Sends a message to a participant.
	 *
	 * @param recipient Who receives the message. Not owned.
	 * @param message The text to send.
	 */
	virtual void sendNotification(Participant* recipient, std::string message) = 0;

	/** @brief Virtual destructor so implementations are deleted correctly. */
	virtual ~NotificationService() {}
};

} // namespace tividy

#endif // NOTIFICATIONSERVICE_H