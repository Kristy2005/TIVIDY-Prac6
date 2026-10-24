/**
 * @file WorkflowHistory.h
 * @brief Audit log of events plus the stack of mementos used for undo.
 */
#ifndef TIVIDY_ENGINE_WORKFLOWHISTORY_H
#define TIVIDY_ENGINE_WORKFLOWHISTORY_H

#include <string>
#include <vector>
#include "engine/WorkflowEvent.h"
#include "engine/WorkflowMemento.h"
#include "engine/WorkflowObserver.h"

namespace tividy {

/** @brief Memento caretaker and append-only audit trail for one workflow instance. */
class WorkflowHistory {
public:
    /** @brief Append an event; assigns its sequence number (1, 2, 3, ...). */
    void record(const WorkflowEvent& event);
    /** @return Every event so far, oldest first. */
    const std::vector<WorkflowEvent>& events() const { return events_; }
    /** @return Events of one type, oldest first. */
    std::vector<WorkflowEvent> eventsOfType(EventType type) const;
    /** @return The audit trail as text, one event per line. */
    std::string report() const;

    /** @brief Store a snapshot taken before a reversible action. */
    void pushSnapshot(const WorkflowMemento& m);
    /** @brief Remove and return the newest snapshot. Throws WorkflowError if there is none. */
    WorkflowMemento popSnapshot();
    size_t snapshotCount() const { return snapshots_.size(); }

private:
    std::vector<WorkflowEvent> events_;
    std::vector<WorkflowMemento> snapshots_;
};

/** @brief Observer that writes every event it hears into a WorkflowHistory. */
class HistoryObserver : public WorkflowObserver {
public:
    explicit HistoryObserver(WorkflowHistory& history) : history_(history) {}
    void update(const WorkflowEvent& event) { history_.record(event); }

private:
    WorkflowHistory& history_;
};

} // namespace tividy

#endif
