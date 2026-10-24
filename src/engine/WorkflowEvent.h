/**
 * @file WorkflowEvent.h
 * @brief The record that travels from a workflow instance to its observers and into the audit log.
 */
#ifndef TIVIDY_ENGINE_WORKFLOWEVENT_H
#define TIVIDY_ENGINE_WORKFLOWEVENT_H

#include <string>

namespace tividy {

/** @brief What happened. */
enum class EventType {
    InstanceCreated,
    InstanceStarted,
    WorkAvailable,
    WorkAssigned,
    WorkCompleted,
    WorkSkipped,
    WorkCancelled,
    RoutingDecision,
    StateChanged,
    Escalated,
    EscalationResolved,
    Undone,
    InvalidAction,
    NotificationFailed,
    WorkflowClosed
};

/** @return A fixed-width style label for @p t, e.g. "WORK_ASSIGNED". */
const char* eventTypeName(EventType t);

/** @brief One thing that happened to one workflow instance. */
struct WorkflowEvent {
    unsigned sequence;        ///< Position in the audit log (assigned by WorkflowHistory).
    EventType type;
    std::string instanceId;
    std::string subject;      ///< Work item id the event is about (may be empty).
    std::string actor;        ///< Who caused it ("system" for automatic actions).
    std::string recipient;    ///< Participant to notify (empty = nobody).
    std::string message;      ///< Human readable detail.

    WorkflowEvent() : sequence(0), type(EventType::InstanceCreated) {}
};

/** @return One-line description such as "#4 WORK_ASSIGNED [intake] by system: Assigned to alice". */
std::string describe(const WorkflowEvent& e);

} // namespace tividy

#endif
