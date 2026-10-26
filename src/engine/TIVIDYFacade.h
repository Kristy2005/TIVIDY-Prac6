#ifndef TIVIDYFACADE_H
#define TIVIDYFACADE_H

#include "WorkflowDefinition.h"
#include "WorkflowInstance.h"
#include "WorkItem.h"


namespace tividy {
class TIVIDYFacade {


public:
	WorkflowDefinition* createWorkflow();

	WorkflowInstance* startWorkflow(WorkflowDefinition* definition);

	void assignWork(WorkflowInstance* instance, WorkItem* item);

	void completeWork(WorkflowInstance* instance, WorkItem* item);

	void escalateWork(WorkflowInstance* instance);

	void closeWorkflow(WorkflowInstance* instance);
};

#endif // TIVIDYFACADE_H

}

