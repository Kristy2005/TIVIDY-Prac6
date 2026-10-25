#include "engine/WorkflowEvent.h"
#include <sstream>

namespace tividy {

const char* eventTypeName(EventType t) {
    switch (t) {
        case EventType::InstanceCreated:    return "INSTANCE_CREATED";
        case EventType::InstanceStarted:    return "INSTANCE_STARTED";
        case EventType::WorkAvailable:      return "WORK_AVAILABLE";
        case EventType::WorkAssigned:       return "WORK_ASSIGNED";
        case EventType::WorkCompleted:      return "WORK_COMPLETED";
        case EventType::WorkSkipped:        return "WORK_SKIPPED";
        case EventType::WorkCancelled:      return "WORK_CANCELLED";
        case EventType::RoutingDecision:    return "ROUTING_DECISION";
        case EventType::StateChanged:       return "STATE_CHANGED";
        case EventType::Escalated:          return "ESCALATED";
        case EventType::EscalationResolved: return "ESCALATION_RESOLVED";
        case EventType::Undone:             return "UNDONE";
        case EventType::InvalidAction:      return "INVALID_ACTION";
        case EventType::NotificationFailed: return "NOTIFICATION_FAILED";
        case EventType::WorkflowClosed:     return "WORKFLOW_CLOSED";
    }
    return "UNKNOWN";
}

std::string describe(const WorkflowEvent& e) {
    std::ostringstream os;
    os << '#' << e.sequence << ' ' << eventTypeName(e.type);
    if (!e.subject.empty()) os << " [" << e.subject << ']';
    if (!e.actor.empty()) os << " by " << e.actor;
    if (!e.message.empty()) os << ": " << e.message;
    return os.str();
}

} // namespace tividy
