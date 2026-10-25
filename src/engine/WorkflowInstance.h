#ifndef WORKFLOWINSTANCE_H
#define WORKFLOWINSTANCE_H

class WorkflowInstance {

private:
	WorkflowState* currentState;
	WorkflowDefinition* definition;
	vector<WorkflowObserver*> observers;

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
