#ifndef WORKFLOWDIRECTOR_H
#define WORKFLOWDIRECTOR_H

 #include "WorkflowBuilder.h"


namespace tividy {
/**
 * @brief Holds the builder used to assemble a workflow definition.
 *
 * The builder pointer is non-owning. In the current implementation,
 * constructWorkflow() only verifies that a builder is set; it does not call
 * any of the builder's construction methods.
 *
 */
	class WorkflowDirector {
	private:
    /** @brief Selected builder; not owned. */
		WorkflowBuilder* builder;

	public:
    /** @brief Construct a director without a builder. */
        WorkflowDirector();
		
	/**
     * @brief Select the builder that the director should use.
     * @param builder Builder to retain without taking ownership; may be null.
     */	
		void setBuilder(WorkflowBuilder* builder);

	/**
     * @brief Validate that a builder is configured.
     *
     * No workflow construction steps are performed yet.
     * @throws std::logic_error If no builder has been configured.
     */
		void constructWorkflow();
	};

}

#endif // WORKFLOWDIRECTOR_H
