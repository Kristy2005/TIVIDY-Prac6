#include "engine/EmailAdapter.h"

namespace tividy {

EmailAdapter::EmailAdapter(ExternalEmailService& service, const std::string& domain)
    : service_(service), domain_(domain) {}

bool EmailAdapter::send(const std::string& recipient, const std::string& subject, const std::string& body) {
    std::string address = recipient.find('@') == std::string::npos ? recipient + "@" + domain_ : recipient;
    std::string raw = "Subject: " + subject + "\n\n" + body;
    return service_.transmit(address, raw) == 0;
}

} // namespace tividy
