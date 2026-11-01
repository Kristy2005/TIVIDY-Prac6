#ifndef HISTORYOBSERVER_H
#define HISTORYOBSERVER_H

#include "WorkflowObserver.h"


namespace tividy {

class WorkflowHistory;
/**
 * @brief Observer pattern: ConcreteObserver. Records the instance's state
 *        each time it changes.
 *
 * On every notification it asks the instance for a memento and stores it in
 * a WorkflowHistory.
 *
 * @par Limitation
 * A memento holds a pointer to the state object, not a copy of the whole
 * workflow, and it does not say what happened. This is therefore not yet the
 * event-by-event audit trail the project requires.
 *
 * @see WorkflowObserver, WorkflowHistory, WorkflowMemento
 */
class HistoryObserver : public WorkflowObserver {
private:
/**
 * @brief History receiving audit events from this observer. Not owned.
 */
WorkflowHistory* history;

public:
	/**
	 * @brief Stores a memento of the instance in the history.
	 *
	 * @param instance The instance that changed. Not stored.
	 */
	virtual void update(WorkflowInstance* instance) override;

	/**
	 * @brief Creates an observer that records into one workflow history.
	 * @param history History to receive events; must not be null.
	 */
	explicit HistoryObserver(WorkflowHistory* history);
};

} // namespace tividy

#endif // HISTORYOBSERVER_H