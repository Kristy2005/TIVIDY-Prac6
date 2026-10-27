#ifndef EMAILADAPTER_H
#define EMAILADAPTER_H

#include "NotificationService.h"
#include "ExternalEmailService.h"
#include <string>

namespace tividy {

/**
 * @brief Adapter pattern: Adapter (object adapter).
 *
 * Makes ExternalEmailService usable as a NotificationService. It holds a
 * pointer to the service (composition, not inheritance) and translates
 * sendNotification(recipient, message) into sendEmail(address, body).
 *
 * @par Translation
 * Participant has no email address, so the recipient's name is used as the
 * address. This is a simplification for the simulated service.
 *
 * @par Ownership
 * The adapter does not own the service. The creator deletes it and keeps it
 * alive while the adapter is used.
 *
 * @see NotificationService, ExternalEmailService
 */
class EmailAdapter : public NotificationService {

public:
	/** @brief The email service this adapter calls. Not owned. */
	ExternalEmailService* service;

	/** @brief Creates an adapter with no service set. */
	EmailAdapter() : service(nullptr) {}

	/**
	 * @brief Sends the message to the recipient through the email service.
	 *
	 * Does nothing if the recipient or the service is null.
	 *
	 * @param recipient Who receives the message. Not owned.
	 * @param message The email body.
	 */
	virtual void sendNotification(Participant* recipient, std::string message) override;
};

} // namespace tividy

#endif // EMAILADAPTER_H