#ifndef WORKFLOWOBSERVER_H
#define WORKFLOWOBSERVER_H

#include "WorkflowInstance.h"

namespace tividy {

class WorkflowInstance;

/**
 * @brief Observer pattern: abstract Observer.
 *
 * Implemented by anything that reacts when a WorkflowInstance changes, such
 * as recording history, showing progress or notifying a participant. The
 * instance (the Subject) keeps a list of WorkflowObserver pointers and knows
 * only this interface, never the concrete observers.
 *
 * @par Pull model
 * update() passes the instance itself and nothing else. An observer reads
 * whatever it needs from the instance, for example getState().
 *
 * @par Ownership
 * The instance keeps the pointers but does not own the observers. Whoever
 * creates an observer deletes it, and should detach it from the instance
 * first, otherwise the instance is left holding a dangling pointer.
 *
 * @par Concrete observers
 * ProgressObserver, HistoryObserver and NotificationObserver.
 *
 * @see WorkflowInstance::attach, WorkflowInstance::detach,
 *      WorkflowInstance::notifyObservers
 */
class WorkflowObserver {

public:
	/**
	 * @brief Called by the instance after its state has changed.
	 *
	 * The change is complete when this runs, so the observer sees the new
	 * state.
	 *
	 * @param instance The instance that changed. Not null. Only valid during
	 *        this call, so do not store the pointer.
	 *
	 * @note Do not change the instance's state from inside update().
	 *       setState() notifies observers, so that would start another
	 *       notification while this one is still running.
	 * @note Do not delete the instance from inside update().
	 * @todo An exception thrown here is not caught by the instance yet, so it
	 *       can stop the remaining observers from being notified.
	 */
	virtual void update(WorkflowInstance* instance) = 0;

	/**
	 * @brief Virtual destructor so a concrete observer is destroyed
	 *        correctly through a WorkflowObserver pointer.
	 */
	virtual ~WorkflowObserver() {}
};

} // namespace tividy

#endif // WORKFLOWOBSERVER_H