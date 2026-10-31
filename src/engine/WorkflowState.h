#ifndef WORKFLOWSTATE_H
#define WORKFLOWSTATE_H

#include "WorkflowInstance.h"

/**
 * @brief State pattern: abstract State.
 *
 * Declares every lifecycle transition a WorkflowInstance can be asked to
 * make. The instance (the Context) never decides whether a transition is
 * allowed. It hands the request to its current state, and that state
 * decides.
 *
 * @par Allowed and rejected transitions
 * Each concrete state implements all five operations. A transition the
 * state allows moves the instance on by calling instance->setState() with
 * the next state. A transition the state does not allow throws
 * std::logic_error and leaves the instance unchanged.
 *
 * @par Ownership
 * WorkflowInstance stores a non-owning pointer to its current state and
 * does not delete state objects. The concrete transitions currently use
 * shared, function-local static state objects. WorkflowMemento stores the
 * same non-owning state pointer; it does not clone or delete the state.
 * Any state pointer passed to WorkflowInstance::setState() must therefore
 * remain valid for as long as an instance or memento may refer to it.
 *
 * @par Concrete states
 * SubmittedState, ActiveState, InProgressState, ResolvedState and
 * ClosedState. ResolvedState and ClosedState are final and reject every
 * transition.
 *
 * @see WorkflowInstance
 */

namespace tividy {

	class WorkflowInstance;

	class WorkflowState {

	public:
	/**
	 * @brief Moves a submitted instance into the Active state.
	 *
	 * @param instance The instance to change. Must not be null.
	 * @throws std::logic_error If this state does not allow activation.
	 */
		virtual void activate(WorkflowInstance* instance) = 0;

	/**
	 * @brief Starts work on an active instance, moving it to In Progress.
	 *
	 * @param instance The instance to change. Must not be null.
	 * @throws std::logic_error If this state does not allow processing to start.
	 */
		virtual void startProcessing(WorkflowInstance* instance) = 0;

	/**
	 * @brief Marks the instance as successfully resolved.
	 *
	 * @param instance The instance to change. Must not be null.
	 * @throws std::logic_error If this state does not allow resolving.
	 */
		virtual void resolve(WorkflowInstance* instance) = 0;

	/**
	 * @brief Closes the instance without resolving it, for example when the
	 *        student withdraws or a final appeal is denied.
	 *
	 * @param instance The instance to change. Must not be null.
	 * @throws std::logic_error If this state does not allow closing.
	 */
		virtual void close(WorkflowInstance* instance) = 0;

	/**
	 * @brief Virtual destructor so concrete classes can be deleted properly.
	 */
		virtual ~WorkflowState(){};
	};
}

#endif // WORKFLOWSTATE_H