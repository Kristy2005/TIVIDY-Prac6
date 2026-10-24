/**
 * @file WorkflowInstance.h
 * @brief One independent run of a WorkflowDefinition.
 */
#ifndef TIVIDY_ENGINE_WORKFLOWINSTANCE_H
#define TIVIDY_ENGINE_WORKFLOWINSTANCE_H

#include <map>
#include <memory>
#include <string>
#include <vector>
#include "engine/Participant.h"
#include "engine/ProgressObserver.h"
#include "engine/WorkflowDefinition.h"
#include "engine/WorkflowHistory.h"
#include "engine/WorkflowMemento.h"
#include "engine/WorkflowObserver.h"
#include "engine/WorkflowState.h"

namespace tividy {

/**
 * @brief A running workflow: its own copy of the work tree, participants, data, state and history.
 *
 * Two instances of the same definition never share mutable data (Prototype gives each its own tree).
 * The public action methods delegate to the current WorkflowState (State pattern), which decides
 * whether the action is allowed. Every change is announced to observers (Observer pattern).
 * Normally callers use Commands (via the Facade) rather than calling these directly.
 */
class WorkflowInstance : public WorkflowSubject {
public:
    WorkflowInstance(const std::string& id, std::shared_ptr<const WorkflowDefinition> definition);
    ~WorkflowInstance();

    const std::string& id() const { return id_; }
    const WorkflowDefinition& definition() const { return *definition_; }
    std::string stateName() const { return state_->name(); }

    // ---- participants and data -------------------------------------------
    /** @brief Register a participant. Throws if the name is taken or the role is not in the definition. */
    Participant& addParticipant(const std::string& name, const std::string& role);
    Participant* findParticipant(const std::string& name);
    std::vector<Participant*> participants();
    void setData(const std::string& key, const std::string& value);
    std::string getData(const std::string& key, const std::string& fallback = "") const;

    // ---- actions (delegated to the current state) -------------------------
    void start(const std::string& actor);
    /** @brief Give @p itemId to @p participant, or let the strategy choose when @p participant is empty. */
    void assign(const std::string& itemId, const std::string& participant, const std::string& actor);
    void complete(const std::string& itemId, const std::string& actor, const std::string& outcome);
    void cancelItem(const std::string& itemId, const std::string& actor, const std::string& reason);
    void escalate(const std::string& actor, const std::string& reason);
    void resolveEscalation(const std::string& actor, const std::string& notes);
    void close(const std::string& actor, const std::string& reason);

    // ---- inspection ---------------------------------------------------------
    /** @return Every work item in definition order. */
    std::vector<WorkflowComponent*> items();
    /** @return The item with this id, or NULL. */
    WorkflowComponent* findItem(const std::string& itemId);
    /** @return Items that are Available or Assigned, most urgent first. */
    std::vector<WorkflowComponent*> availableWork();
    int totalItems() const;
    int finishedItems() const;
    bool allWorkFinished() const;
    int escalationCount() const { return escalationCount_; }
    /** @return Participant currently responsible for an open escalation ("" if none). */
    const std::string& escalatedTo() const { return escalatedTo_; }
    WorkflowHistory& history() { return history_; }
    const WorkflowHistory& history() const { return history_; }
    const ProgressObserver& progress() const { return progress_; }
    /** @return The observer that writes to the audit log (useful as a failure sink). */
    WorkflowObserver& historyObserver() { return historyObserver_; }
    /** @return Multi-line status text: state, progress and every item. */
    std::string describe();

    // ---- Memento ------------------------------------------------------------
    WorkflowMemento createMemento();
    void restore(const WorkflowMemento& m);

    /** @brief Announce something that happened (audit log, notifications, progress all hear it). */
    void recordEvent(EventType type, const std::string& subject, const std::string& actor,
                     const std::string& recipient, const std::string& message);

private:
    friend class WorkflowState;
    WorkflowInstance(const WorkflowInstance&);
    WorkflowInstance& operator=(const WorkflowInstance&);

    // Implementations called by the states.
    void doStart(const std::string& actor);
    void doAssign(const std::string& itemId, const std::string& participant, const std::string& actor);
    void doComplete(const std::string& itemId, const std::string& actor, const std::string& outcome);
    void doCancelItem(const std::string& itemId, const std::string& actor, const std::string& reason);
    void doEscalate(const std::string& actor, const std::string& reason);
    void doResolveEscalation(const std::string& actor, const std::string& notes);
    void doClose(const std::string& actor, const std::string& reason);
    void transitionTo(WorkflowState& next, const std::string& actor, const std::string& reason);

    // Routing engine.
    void advance(const std::string& actor);
    void assignTo(WorkflowComponent& item, Participant& p, const std::string& actor);
    void release(WorkflowComponent& item);
    WorkflowComponent& requireItem(const std::string& itemId);

    std::string id_;
    std::shared_ptr<const WorkflowDefinition> definition_;
    std::unique_ptr<WorkflowComponent> tree_;
    std::map<std::string, Participant> participants_;
    std::map<std::string, std::string> data_;
    WorkflowState* state_;
    int escalationCount_;
    std::string escalatedTo_;
    std::string escalationReason_;
    WorkflowHistory history_;
    HistoryObserver historyObserver_;
    ProgressObserver progress_;
};

} // namespace tividy

#endif
