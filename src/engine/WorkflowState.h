/**
 * @file WorkflowState.h
 * @brief State pattern: what a workflow instance may do depends on its lifecycle state.
 *
 * States are stateless, so one shared object per state is used (accessed via instance()).
 * Operations that are not allowed in a state throw InvalidTransition (the default).
 *
 * Lifecycle:  Submitted -> Active -> InProgress <-> Escalated
 *                                       |
 *                                    Resolved      (any state) -> Closed
 */
#ifndef TIVIDY_ENGINE_WORKFLOWSTATE_H
#define TIVIDY_ENGINE_WORKFLOWSTATE_H

#include <string>
#include "engine/Types.h"

namespace tividy {

class WorkflowInstance;

/** @brief Abstract state. Each hook either performs the action or rejects it. */
class WorkflowState {
public:
    virtual ~WorkflowState();
    /** @return The state's name, e.g. "InProgress". */
    virtual std::string name() const = 0;

    // ---- actions requested by users (via Commands) -----------------------
    virtual void start(WorkflowInstance& w, const std::string& actor);
    virtual void assign(WorkflowInstance& w, const std::string& itemId, const std::string& participant,
                        const std::string& actor);
    virtual void complete(WorkflowInstance& w, const std::string& itemId, const std::string& actor,
                          const std::string& outcome);
    virtual void cancelItem(WorkflowInstance& w, const std::string& itemId, const std::string& actor,
                            const std::string& reason);
    virtual void escalate(WorkflowInstance& w, const std::string& actor, const std::string& reason);
    virtual void resolveEscalation(WorkflowInstance& w, const std::string& actor, const std::string& notes);
    virtual void close(WorkflowInstance& w, const std::string& actor, const std::string& reason);

    // ---- hooks called by the instance as work progresses ------------------
    /** @brief Some item has just been assigned. */
    virtual void onWorkAssigned(WorkflowInstance& w);
    /** @brief Every work item is now terminal. */
    virtual void onAllWorkFinished(WorkflowInstance& w);

    /** @return The shared state object called @p name. Throws WorkflowError for unknown names. */
    static WorkflowState& byName(const std::string& name);

protected:
    /** @brief Throw InvalidTransition: @p action is not allowed in this state. */
    void reject(const WorkflowInstance& w, const char* action) const;

    // Forwarders to WorkflowInstance's private implementation (WorkflowState is its friend).
    static void performStart(WorkflowInstance& w, const std::string& actor);
    static void performAssign(WorkflowInstance& w, const std::string& itemId, const std::string& participant,
                              const std::string& actor);
    static void performComplete(WorkflowInstance& w, const std::string& itemId, const std::string& actor,
                                const std::string& outcome);
    static void performCancelItem(WorkflowInstance& w, const std::string& itemId, const std::string& actor,
                                  const std::string& reason);
    static void performEscalate(WorkflowInstance& w, const std::string& actor, const std::string& reason);
    static void performResolveEscalation(WorkflowInstance& w, const std::string& actor, const std::string& notes);
    static void performClose(WorkflowInstance& w, const std::string& actor, const std::string& reason);
    static void transition(WorkflowInstance& w, WorkflowState& next, const std::string& actor,
                           const std::string& reason);
};

#define TIVIDY_DECLARE_STATE(Class)                                  \
    class Class : public WorkflowState {                             \
    public:                                                          \
        static Class& instance();                                    \
        std::string name() const;

/** @brief Created, not started. Can be started or closed. */
TIVIDY_DECLARE_STATE(SubmittedState)
    void start(WorkflowInstance& w, const std::string& actor);
    void close(WorkflowInstance& w, const std::string& actor, const std::string& reason);
};

/** @brief Running, with no work assigned yet. */
TIVIDY_DECLARE_STATE(ActiveState)
    void assign(WorkflowInstance& w, const std::string& itemId, const std::string& participant, const std::string& actor);
    void cancelItem(WorkflowInstance& w, const std::string& itemId, const std::string& actor, const std::string& reason);
    void close(WorkflowInstance& w, const std::string& actor, const std::string& reason);
    void onWorkAssigned(WorkflowInstance& w);
    void onAllWorkFinished(WorkflowInstance& w);
};

/** @brief Work is being done. The only state in which items can be completed. */
TIVIDY_DECLARE_STATE(InProgressState)
    void assign(WorkflowInstance& w, const std::string& itemId, const std::string& participant, const std::string& actor);
    void complete(WorkflowInstance& w, const std::string& itemId, const std::string& actor, const std::string& outcome);
    void cancelItem(WorkflowInstance& w, const std::string& itemId, const std::string& actor, const std::string& reason);
    void escalate(WorkflowInstance& w, const std::string& actor, const std::string& reason);
    void close(WorkflowInstance& w, const std::string& actor, const std::string& reason);
    void onAllWorkFinished(WorkflowInstance& w);
};

/** @brief Handed to a higher authority; normal work is frozen until the escalation is resolved. */
TIVIDY_DECLARE_STATE(EscalatedState)
    void resolveEscalation(WorkflowInstance& w, const std::string& actor, const std::string& notes);
    void close(WorkflowInstance& w, const std::string& actor, const std::string& reason);
};

/** @brief All work finished. Only closing is possible. */
TIVIDY_DECLARE_STATE(ResolvedState)
    void close(WorkflowInstance& w, const std::string& actor, const std::string& reason);
};

/** @brief Terminal: nothing is allowed any more. */
TIVIDY_DECLARE_STATE(ClosedState)
};

#undef TIVIDY_DECLARE_STATE

} // namespace tividy

#endif
