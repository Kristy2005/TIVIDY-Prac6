#ifndef WORKFLOWDIRECTOR_H
#define WORKFLOWDIRECTOR_H

// #include "WorkflowBuilder.h"
class WorkflowBuilder;


namespace tividy {
	class WorkflowDirector {

	private:
		WorkflowBuilder* builder;

	public:
		void setBuilder(WorkflowBuilder* builder);

		void constructWorkflow();
	};

}

#endif // WORKFLOWDIRECTOR_H
