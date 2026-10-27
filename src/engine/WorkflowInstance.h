#ifndef WORKFLOWINSTANCE_H
#define WORKFLOWINSTANCE_H

#include "WorkflowDefinition.h"
#include "WorkflowMemento.h"
#include <vector>

namespace tividy {

class WorkflowObserver;
class WorkflowState;

/**
 * @brief One running execution of a WorkflowDefinition.
 *
 * Plays three pattern roles:
 * - State pattern: Context. Holds the current WorkflowState and changes it
 *   as the case moves through its lifecycle.
 * - Observer pattern: Subject. Keeps a list of observers and notifies them
 *   when the state changes.
 * - Memento pattern: Originator. Creates mementos of its state and restores
 *   from them.
 *
 * @par Lifecycle
 * Submitted, Active, In Progress, Resolved or Closed. The lifecycle
 * operations below move the instance between these states.
 *
 * @par Ownership
 * - The instance does not own its definition. The definition must outlive
 *   every instance created from it.
 * - The instance does not own its observers. The creator deletes them and
 *   should detach them first.
 * - The instance never deletes a state it has replaced (see setState()).
 *
 * @warning Not safe to copy, because it holds raw pointers. Do not pass it
 *          by value or assign it.
 *
 * @see WorkflowDefinition, WorkflowState, WorkflowObserver, WorkflowMemento
 */
class WorkflowInstance {
private:
    /** @brief The state the instance is in now. Never null after construction. */
    WorkflowState* currentState;

    /** @brief The blueprint this instance was created from. Not owned. */
    WorkflowDefinition* definition;

    /** @brief Observers to notify on a state change. Not owned. */
    std::vector<WorkflowObserver*> observers;

public:
    explicit WorkflowInstance(WorkflowDefinition* definition = nullptr);
    WorkflowInstance(const WorkflowInstance&) = delete;
    WorkflowInstance& operator=(const WorkflowInstance&) = delete;
    ~WorkflowInstance();

    /**
     * @brief Returns the current state.
     *
     * Observers call this to read the new state when they are notified.
     *
     * @return The current state. Owned by the engine, not by the caller.
     */
    WorkflowState* getState() const;

    /**
     * @brief Replaces the current state and notifies the observers.
     *
     * Called by the state classes to make a transition. The observers are
     * notified after the new state is in place.
     *
     * @param state The new state. Must not be null.
     *
     * @warning The previous state is not deleted. A saved memento may still
     *          point at it, so deleting it would leave that memento dangling.
     *          Until state ownership is redesigned, replaced states stay in
     *          memory.
     */
    void setState(WorkflowState* state);

    /**
     * @brief Submits the case, putting the instance in the Submitted state.
     *
     * The first step of the lifecycle. TIVIDYFacade::startWorkflow() calls
     * it, then activate().
     */
    void submit();

    /** @brief Moves the instance from Submitted to Active. */
    void activate();

    /** @brief Starts work on the case, moving the instance to In Progress. */
    void startProcessing();

    /**
     * @brief Escalates the case to a higher level.
     *
     * Only allowed while the instance is In Progress. The request is passed
     * to the definition, which sends it through the escalation chain of
     * handlers.
     *
     * @throws std::logic_error If the instance is not In Progress.
     */
    void escalate();

    /**
     * @brief Takes a snapshot of the current state.
     *
     * Used by commands before a change, so the change can be undone.
     *
     * @return A new memento. The caller must delete it or give it to an
     *         owner, for example a WorkflowHistory.
     *
     * @warning The memento stores a pointer to the state object, not a copy
     *          of the whole instance. It does not capture item progress.
     */
    WorkflowMemento* createMemento();

    /**
     * @brief Returns the instance to the state stored in a memento.
     *
     * The observers and the definition are not part of a memento and are
     * left as they are.
     *
     * @param memento The snapshot to restore. Must not be null.
     */
    void restore(WorkflowMemento* memento);

    /** @brief Marks the case as successfully resolved. */
    void resolve();

    /**
     * @brief Closes the case without resolving it.
     *
     * Used when the student retracts the complaint or the request is
     * rejected.
     */
    void close();

    /**
     * @brief Calls update() on every registered observer.
     *
     * Observers are called in the order they were attached. Called by
     * setState() after the state has changed.
     */
    void notifyObservers();

    /**
     * @brief Registers an observer so it is notified of state changes.
     *
     * The instance does not take ownership of the observer.
     *
     * @param observer The observer to add. Must not be null.
     */
    void attach(WorkflowObserver* observer);

    /**
     * @brief Removes an observer so it is no longer notified.
     *
     * Call this before deleting the observer. The observer itself is not
     * deleted.
     *
     * @param observer The observer to remove.
     */
    void detach(WorkflowObserver* observer);
};

} // namespace tividy

#endif // WORKFLOWINSTANCE_H