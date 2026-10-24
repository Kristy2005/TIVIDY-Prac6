/**
 * @file NotificationObserver.h
 * @brief Observer that forwards events addressed to a participant to a NotificationService.
 */
#ifndef TIVIDY_ENGINE_NOTIFICATIONOBSERVER_H
#define TIVIDY_ENGINE_NOTIFICATIONOBSERVER_H

#include "engine/NotificationService.h"
#include "engine/WorkflowObserver.h"

namespace tividy {

/**
 * @brief Sends a notification for every event that names a recipient.
 *
 * If the channel fails, the failure is reported to @p failureSink (normally the
 * audit-log observer) and the workflow carries on: a broken email gateway must
 * never undo an escalation.
 */
class NotificationObserver : public WorkflowObserver {
public:
    NotificationObserver(NotificationService& service, WorkflowObserver* failureSink = 0);
    void update(const WorkflowEvent& event);
    /** @return Number of messages that could not be delivered. */
    int failureCount() const { return failures_; }

private:
    NotificationService& service_;
    WorkflowObserver* failureSink_;
    int failures_;
};

} // namespace tividy

#endif
