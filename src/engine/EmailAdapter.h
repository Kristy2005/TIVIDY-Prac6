/**
 * @file EmailAdapter.h
 * @brief Adapter: makes ExternalEmailService usable as a NotificationService.
 */
#ifndef TIVIDY_ENGINE_EMAILADAPTER_H
#define TIVIDY_ENGINE_EMAILADAPTER_H

#include "engine/ExternalEmailService.h"
#include "engine/NotificationService.h"

namespace tividy {

/** @brief Translates send(recipient, subject, body) into transmit(address, rawMessage). */
class EmailAdapter : public NotificationService {
public:
    /**
     * @param service Gateway to adapt (not owned; must outlive the adapter).
     * @param domain  Appended to recipients that are not already addresses.
     */
    explicit EmailAdapter(ExternalEmailService& service, const std::string& domain = "tividy.example");

    bool send(const std::string& recipient, const std::string& subject, const std::string& body);

private:
    ExternalEmailService& service_;
    std::string domain_;
};

} // namespace tividy

#endif
