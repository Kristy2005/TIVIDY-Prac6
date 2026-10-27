#ifndef EXTERNALEMAILSERVICE_H
#define EXTERNALEMAILSERVICE_H

#include <string>

/**
 * @brief Adapter pattern: Adaptee. A stand-in for an external email system.
 *
 * It has its own interface, different from NotificationService. It does not
 * connect to a real provider: it prints the email to the console, so a
 * notification can be seen without an email account.
 *
 * @see EmailAdapter
 */

namespace tividy {
	class ExternalEmailService {


	public:

/**
 * @brief Prints an email to the console instead of sending it.
 *
 * @param address Where the email would be sent.
 * @param body The text of the email.
 */
		void sendEmail(std::string address, std::string body);
	};

}

#endif  // EXTERNALEMAILSERVICE_H
