#ifndef TIVIDYFACADE_H
#define TIVIDYFACADE_H

#include "WorkflowDefinition.h"
#include "WorkflowInstance.h"
#include "Participant.h"

#include <vector>

namespace tividy {
/**
 * @brief Facade exposing common workflow operations through a small interface.
 *
 * The facade does not own the definitions, instances, or work items supplied
 * by callers. createWorkflow() and startWorkflow() return allocated objects
 * that the caller must delete.
 *
 */
	class TIVIDYFacade {
	public:
	/**
     * @brief Allocate an empty workflow definition.
     * @return Newly allocated definition; caller owns the returned pointer.
     */
		WorkflowDefinition* createWorkflow();

	/**
     * @brief Create an instance from a definition, then submit and activate it.
     * @param definition Definition to use; must not be null and must outlive the instance.
     * @return Newly allocated workflow instance; caller owns it.
     * @throws std::invalid_argument If @p definition is null.
     */
		WorkflowInstance* startWorkflow(WorkflowDefinition* definition);

	/**
     * @brief Execute a work item without recording a participant assignment.
     *
     * This overload is useful when the caller manages assignment separately.
     * It does not update participant workload. Use the overload that accepts
     * candidate participants when the Facade should select and record an
     * instance-aware assignment.
     *
     * @param instance Workflow instance argument; must not be null.
     * @param item Work item to execute; must not be null.
     * @throws std::invalid_argument If either argument is null.
     */
		void assignWork(WorkflowInstance* instance, WorkItem* item);

	/**
     * @brief Mark an item complete for an instance without a participant.
     *
     * Completion is recorded in the instance's progress records. The workflow
     * is resolved only when all tracked items in that instance are complete.
     * Because no participant is supplied, this overload cannot update any
     * participant's workload. Use the overload that accepts a participant
     * when completing a recorded assignment.
     *
     * @param instance Workflow instance; must not be null.
     * @param item Work item to mark complete; must belong to the instance.
     * @throws std::invalid_argument If either argument is null or the item
     *         does not belong to the instance.
     */
		void completeWork(WorkflowInstance* instance, WorkItem* item);

	/**
     * @brief Request escalation of a workflow instance.
     * @param instance Instance to escalate; must not be null and must be in progress.
     * @throws std::invalid_argument If @p instance is null.
     * @throws std::logic_error If escalation is not allowed or no handler is configured.
     */
		void escalateWork(WorkflowInstance* instance);

	/**
     * @brief Close a workflow instance.
     * @param instance Instance to close; must not be null and the state must allow closure.
     * @throws std::invalid_argument If @p instance is null.
     * @throws std::logic_error If the state rejects closure.
     */
		void closeWorkflow(WorkflowInstance* instance);

        /**
         * @brief Select and record an instance-aware participant assignment.
         * @param instance Workflow instance to which the item belongs.
         * @param item Work item to assign.
         * @param participants Candidate participants passed to the configured strategy.
         * @return Selected participant, or nullptr when no suitable candidate exists.
         * @throws std::invalid_argument If the instance or item is null.
         * @throws std::logic_error If the instance has no workflow definition.
         */
        Participant* assignWork(
            WorkflowInstance* instance,
            WorkItem* item,
            std::vector<Participant*> participants
        );

        /**
         * @brief Complete an instance-aware assignment and update participant workload.
         * @param instance Workflow instance associated with the assignment.
         * @param item Work item being completed.
         * @param participant Participant who holds the assignment.
         * @throws std::invalid_argument If any argument is null.
         */
        void completeWork(
            WorkflowInstance* instance,
            WorkItem* item,
            Participant* participant
        );
	};


} // namespace tividy

#endif // TIVIDYFACADE_H
