#include "engine/NotificationObserver.h"

namespace tividy {

NotificationObserver::NotificationObserver(NotificationService& service, WorkflowObserver* failureSink)
    : service_(service), failureSink_(failureSink), failures_(0) {}

void NotificationObserver::update(const WorkflowEvent& event) {
    if (event.recipient.empty()) return;
    std::string subject = std::string("[") + event.instanceId + "] " + eventTypeName(event.type);
    if (service_.send(event.recipient, subject, event.message)) return;

    ++failures_;
    if (failureSink_) {
        WorkflowEvent failure;
        failure.type = EventType::NotificationFailed;
        failure.instanceId = event.instanceId;
        failure.subject = event.subject;
        failure.actor = "system";
        failure.message = "Could not notify " + event.recipient + " about " + eventTypeName(event.type);
        failureSink_->update(failure);
    }
}

} // namespace tividy
