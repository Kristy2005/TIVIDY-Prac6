#include "EmailAdapter.h"
#include <stdexcept>
namespace tividy {
void EmailAdapter::sendNotification(Participant* recipient, std::string message) {
    if (!recipient) throw std::invalid_argument("EmailAdapter: recipient cannot be null");
    if (!service) throw std::logic_error("EmailAdapter: email service is not configured");
    service->sendEmail(recipient->getName(), message);
}
}
