#ifndef WORKFLOWMEMENTO_H
#define WORKFLOWMEMENTO_H

namespace tividy {

class WorkflowState;
/**
 * @brief Stores a workflow-state pointer for later restoration.
 *
 * The current implementation does not clone the state: setState() stores the
 * exact pointer, and the destructor deletes the stored pointer. Consequently,
 * the supplied state must be uniquely owned by the memento. Passing a shared
 * or static state pointer (as WorkflowInstance::createMemento() currently
 * does) can delete a state still in use and can cause double deletion.
 *
 */

class WorkflowMemento {
private:
    /** @brief State pointer released by the destructor; ownership is assumed. */
    WorkflowState* state;

public:
    /** @brief Construct a memento with no stored state. */
    WorkflowMemento();

    /**
     * @brief Store a state pointer without cloning it.
     *
     * This class assumes ownership of @p state and deletes it at destruction.
     * Replacing an already stored pointer does not first delete the old one.
     * @param state State pointer to retain; ownership must be compatible with
     *        the destructor's delete operation.
     */
    void setState(WorkflowState* state);

    /**
     * @brief Get the stored state pointer.
     * @return The stored pointer, or null if none has been set; not a copy.
     */
    WorkflowState* getState() const;

    /** @brief Delete the state pointer currently stored by this memento. */
    ~WorkflowMemento();

    // WorkflowMemento(const WorkflowMemento&) = delete;
    
    // WorkflowMemento& operator=(const WorkflowMemento&) = delete;
    };

}

#endif // WORKFLOWMEMENTO_H

