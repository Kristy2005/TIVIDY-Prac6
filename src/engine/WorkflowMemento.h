#ifndef WORKFLOWMEMENTO_H
#define WORKFLOWMEMENTO_H

#include <ctime>

namespace tividy {

class WorkflowState;

/**
 * @brief Stores a non-owning reference to a workflow state for later restoration.
 *
 * WorkflowInstance uses shared, stateless state objects (currently function-
 * local static objects in the concrete state transitions). A memento therefore
 * keeps the same state pointer as the instance; it does not clone or own that
 * object. Destroying a memento must never delete the state it references.
 *
 * @warning This snapshot contains only the WorkflowState pointer. It does not
 *          preserve workflow work-item progress, observers, or the definition.
 *
 * @see WorkflowInstance, WorkflowState
 */
class WorkflowMemento {
private:
    /** @brief Non-owning pointer to the saved shared state. */
    WorkflowState* state;
    /** @brief The time at which this memento was created. */
    std::time_t timestamp;

public:
    /** @brief Construct a memento with no saved state. */
    WorkflowMemento();

    /**
     * @brief Store a non-owning pointer to a state without cloning it.
     * @param state State to remember; may be null to clear the snapshot.
     *        The state must outlive any restoration that uses this memento.
     */
    void setState(WorkflowState* state);

    /**
     * @brief Get the saved state pointer.
     * @return The original pointer, or null if no state has been saved.
     */
    WorkflowState* getState() const;

    /**
     * @brief Get the timestamp of when this memento was created.
     * @return The time at which the memento was constructed.
     */
    std::time_t getTimestamp() const;

    /** @brief Destroy the snapshot without deleting its non-owned state. */
    ~WorkflowMemento();
};

} // namespace tividy

#endif // WORKFLOWMEMENTO_H
