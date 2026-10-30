#ifndef INPROGRESSSTATE_H
#define INPROGRESSSTATE_H

#include "WorkflowState.h"

/**
 * @brief State pattern: ConcreteState (In Progress).
 *
 * The state of a case that is being worked on.
 *
 * @par Transitions
 * - resolve(): allowed, moves the instance to ResolvedState.
 * - close(): allowed, moves the instance to ClosedState. Used when the
 *   student retracts the complaint or the request is rejected.
 * - activate(), startProcessing(): not allowed in this state. Each throws
 *   std::logic_error and leaves the instance unchanged.
 *
 * Escalation is requested separately through WorkflowInstance::escalate().
 * The current escalation-handler chain does not change the workflow state.
 *
 * @par Guards
 * The conditions in the state diagram (cannot handle at the current level,
 * successfully resolved, retraction, rejected) are checked by the caller
 * before it calls the operation. This class only decides whether the
 * operation is allowed in this state.
 *
 * @par Data
 * Holds no data of its own, so it is safe to share one object between
 * instances.
 *
 * @see WorkflowState, ResolvedState, ClosedState,
 *      WorkflowInstance
 */

namespace tividy {
class InProgressState : public WorkflowState {

public:
	/**
	 * @brief Not allowed: the instance is already past activation.
	 *
	 * @param instance The instance. Not changed.
	 * @throws std::logic_error Always.
	 */
	virtual void activate(WorkflowInstance* instance) override;

	/**
	 * @brief Not allowed: work has already started.
	 *
	 * @param instance The instance. Not changed.
	 * @throws std::logic_error Always.
	 */
	virtual void startProcessing(WorkflowInstance* instance) override;


	/**
	 * @brief Marks the case as successfully resolved, moving the instance
	 *        to Resolved.
	 *
	 * @param instance The instance to change. Must not be null.
	 * @throws std::invalid_argument If @p instance is null.
	 * 
	 */
	virtual void resolve(WorkflowInstance* instance) override;

	/**
	 * @brief Closes the case without resolving it, moving the instance to
	 *        Closed.
	 *
	 * Used when the student retracts the complaint or the request is
	 * rejected.
	 *
	 * @param instance The instance to change. Must not be null.
	 * @throws std::invalid_argument If @p instance is null.
	 */
	virtual void close(WorkflowInstance* instance) override;

};


} // namespace tividy

#endif // INPROGRESSSTATE_H
