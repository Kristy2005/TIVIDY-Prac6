#ifndef ACTIVESTATE_H
#define ACTIVESTATE_H

#include "WorkflowState.h"

/**
 * @brief State pattern: ConcreteState (Active).
 *
 * The state of a case that has been reviewed and is ready to be worked on.
 *
 * @par Transitions
 * - startProcessing(): allowed, moves the instance to InProgressState.
 * - close(): allowed, moves the instance to ClosedState. Used when the
 *   student retracts the complaint before work starts.
 * - activate(), escalate(), resolve(): not allowed in this state. Each
 *   throws std::logic_error and leaves the instance unchanged.
 *
 * @par Data
 * Holds no data of its own, so it is safe to share one object between
 * instances.
 *
 * @see WorkflowState, InProgressState, ClosedState, WorkflowInstance
 */

namespace tividy {
	class ActiveState : public WorkflowState {

	public:
		/**
		 * @brief Not allowed: the instance is already active.
		 *
		 * @param instance The instance. Not changed.
		 * @throws std::logic_error Always.
		 */
		virtual void activate(WorkflowInstance* instance) override;

		/**
		 * @brief Starts work on the case, moving the instance to In Progress.
		 *
		 * @param instance The instance to change. Must not be null.
		 */
		virtual void startProcessing(WorkflowInstance* instance) override;

		/**
		 * @brief Not allowed: a case cannot be resolved before work has started.
		 *
		 * @param instance The instance. Not changed.
		 * @throws std::logic_error Always.
		 */
		virtual void resolve(WorkflowInstance* instance) override;

		/**
		 * @brief Closes the instance because the student retracted the
		 *        complaint.
		 *
		 * The caller checks that the retraction happened before calling this.
		 *
		 * @param instance The instance to change. Must not be null.
		 */
		virtual void close(WorkflowInstance* instance) override;

	};


} // namespace tividy

#endif //ACTIVESTATE_H
