#ifndef CLOSEDSTATE_H
#define CLOSEDSTATE_H

#include "WorkflowState.h"

/**
 * @brief State pattern: ConcreteState (Closed).
 *
 * The final state of a workflow instance that has been closed.
 * Rejects all further transitions.
 *
 * @par Data
 * Holds no data of its own, so it is safe to share one object between
 * instances.
 *
 * @see WorkflowState, WorkflowInstance
 */

namespace tividy {
class ClosedState : public WorkflowState {

public:
	/**
	 * @brief Not allowed from closed state.
	 *
	 * @param instance The instance. Not changed.
	 * @throws std::logic_error Always.
	 */
	virtual void activate(WorkflowInstance* instance) override;

	/**
	 * @brief Not allowed from closed state.
	 *
	 * @param instance The instance. Not changed.
	 * @throws std::logic_error Always.
	 */
	virtual void startProcessing(WorkflowInstance* instance) override;

	/**
	 * @brief Not allowed from closed state.
	 *
	 * @param instance The instance. Not changed.
	 * @throws std::logic_error Always.
	 */
	virtual void resolve(WorkflowInstance* instance) override;

	/**
	 * @brief Not allowed from closed state.
	 *
	 * @param instance The instance. Not changed.
	 * @throws std::logic_error Always.
	 */
	virtual void close(WorkflowInstance* instance) override;

	virtual WorkflowState* clone() const override { return new ClosedState(); }

};

} // namespace tividy

#endif // CLOSEDSTATE_H

