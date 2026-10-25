/**
 * @file NotificationService.h
 * @brief Adapter pattern: the Target interface the engine uses to tell people things.
 */
#ifndef TIVIDY_ENGINE_NOTIFICATIONSERVICE_H
#define TIVIDY_ENGINE_NOTIFICATIONSERVICE_H

#include <string>

namespace tividy {

/** @brief What the engine expects from any notification channel. The engine never sees a foreign API. */
class NotificationService {
public:
    virtual ~NotificationService();
    /**
     * @brief Send a message to a participant.
     * @return true on success, false if the channel failed (never throws).
     */
    virtual bool send(const std::string& recipient, const std::string& subject, const std::string& body) = 0;
};

} // namespace tividy

#endif
