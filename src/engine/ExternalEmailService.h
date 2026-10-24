/**
 * @file ExternalEmailService.h
 * @brief Adaptee: a stand-in for a third-party email system with its own, different interface.
 *
 * This is a documented test stub (the brief allows stubs for external services).
 * It can be switched offline to exercise the failure path.
 */
#ifndef TIVIDY_ENGINE_EXTERNALEMAILSERVICE_H
#define TIVIDY_ENGINE_EXTERNALEMAILSERVICE_H

#include <string>
#include <vector>

namespace tividy {

/** @brief Pretend email gateway: takes an address and a raw message, returns a numeric status code. */
class ExternalEmailService {
public:
    /** @brief One message that was accepted by the gateway. */
    struct Mail {
        std::string address;
        std::string raw;
    };

    ExternalEmailService() : online_(true) {}

    /** @return 0 on success, 503 if the gateway is offline. */
    int transmit(const std::string& address, const std::string& raw);

    void setOnline(bool online) { online_ = online; }
    bool isOnline() const { return online_; }
    /** @return Every message accepted so far. */
    const std::vector<Mail>& outbox() const { return outbox_; }

private:
    bool online_;
    std::vector<Mail> outbox_;
};

} // namespace tividy

#endif
