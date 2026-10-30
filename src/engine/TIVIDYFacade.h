#ifndef TIVIDYFACADE_H
#define TIVIDYFACADE_H

#include "WorkflowDefinition.h"
#include "WorkflowInstance.h"
class WorkItem;

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
     * @brief Execute a work item's execute() operation.
     *
     * The instance is only checked for null; it is not otherwise used, and no
     * participant assignment is performed here.
     * @param instance Workflow instance argument; must not be null.
     * @param item Work item to execute; must not be null.
     * @throws std::invalid_argument If either argument is null.
     */
		void assignWork(WorkflowInstance* instance, WorkItem* item);

	/**
     * @brief Mark a work item complete and request workflow resolution.
     *
     * The current implementation resolves the workflow immediately; it does
     * not check whether every work item in the workflow has completed.
     * @param instance Workflow instance to resolve; must not be null.
     * @param item Work item to mark complete; must not be null.
     * @throws std::invalid_argument If either argument is null.
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
	};

	
} // namespace tividy

#endif // TIVIDYFACADE_H
