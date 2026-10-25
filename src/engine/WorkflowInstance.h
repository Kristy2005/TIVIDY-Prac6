#ifndef WORKFLOWINSTANCE_H
#define WORKFLOWINSTANCE_H

#include "WorkflowDefinition.h"
#include "WorkflowObserver.h"
#include "WorkflowMemento.h"

#include <vector>

class WorkflowInstance {

private:
	WorkflowState* currentState;
	WorkflowDefinition* definition;
	std::vector<WorkflowObserver*> observers;

public:
	void setState(WorkflowState* state);

	void submit();

	void activate();

	void startProcessing();

	void escalate();

	WorkflowMemento* createMemento();

	void restore(WorkflowMemento* memento);

	void resolve();

	void close();

	void notifyObservers();

	void attach(WorkflowObserver* observer);

	void detach(WorkflowObserver* observer);
};

#endif
