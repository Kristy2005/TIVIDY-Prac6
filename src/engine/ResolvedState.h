#ifndef RESOLVEDSTATE_H
#define RESOLVEDSTATE_H

#include "WorkflowState.h"

/**
 * @brief State pattern: ConcreteState (Resolved).
 *
 * The final state of a workflow instance that has been successfully resolved.
 * Rejects all further transitions.
 *
 * @par Data
 * Holds no data of its own, so it is safe to share one object between
 * instances.
 *
 * @see WorkflowState, WorkflowInstance
 */

namespace tividy {
class ResolvedState : public WorkflowState {

public:
	/**
	 * @brief Not allowed from resolved state.
	 *
	 * @param instance The instance. Not changed.
	 * @throws std::logic_error Always.
	 */
	virtual void activate(WorkflowInstance* instance) override;

	/**
	 * @brief Not allowed from resolved state.
	 *
	 * @param instance The instance. Not changed.
	 * @throws std::logic_error Always.
	 */
	virtual void startProcessing(WorkflowInstance* instance) override;

	/**
	 * @brief Not allowed from resolved state.
	 *
	 * @param instance The instance. Not changed.
	 * @throws std::logic_error Always.
	 */
	virtual void resolve(WorkflowInstance* instance) override;

	/**
	 * @brief Not allowed from resolved state.
	 *
	 * @param instance The instance. Not changed.
	 * @throws std::logic_error Always.
	 */
	virtual void close(WorkflowInstance* instance) override;
};

} // namespace tividy

#endif // RESOLVEDSTATE_H
