#ifndef SUBMITTEDSTATE_H
#define SUBMITTEDSTATE_H

#include "WorkflowState.h"

/**
 * @brief State pattern: ConcreteState (Submitted).
 *
 * The state of a newly submitted case that has not been reviewed yet.
 *
 * @par Transitions
 * - activate(): allowed, moves the instance to ActiveState.
 * - startProcessing(), escalate(), resolve(), close(): not allowed in this
 *   state. Each throws std::logic_error and leaves the instance unchanged.
 *
 * @par Data
 * Holds no data of its own, so it is safe to share one object between
 * instances.
 *
 * @see WorkflowState, ActiveState, WorkflowInstance
 */

namespace tividy {
class SubmittedState : public WorkflowState {

public:
	/**
	 * @brief Moves the instance to the Active state.
	 *
	 * @param instance The instance to change. Must not be null.
	 */
	virtual void activate(WorkflowInstance* instance) override;

	/**
	 * @brief Not allowed: a submitted case must be activated first.
	 *
	 * @param instance The instance. Not changed.
	 * @throws std::logic_error Always.
	 */
	virtual void startProcessing(WorkflowInstance* instance) override;
	
	/**
	 * @brief Not allowed: a case cannot be resolved before work has
	 *        started.
	 *
	 * @param instance The instance. Not changed.
	 * @throws std::logic_error Always.
	 */
	virtual void resolve(WorkflowInstance* instance) override;

	/**
	 * @brief Not allowed from this state.
	 *
	 * @param instance The instance. Not changed.
	 * @throws std::logic_error Always.
	 */
	virtual void close(WorkflowInstance* instance) override;

	virtual WorkflowState* clone() const override { return new SubmittedState(); }

};


} // namespace tividy

#endif // SUBMITTEDSTATE_H
