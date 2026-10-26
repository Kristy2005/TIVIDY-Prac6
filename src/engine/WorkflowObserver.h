#ifndef WORKFLOWOBSERVER_H
#define WORKFLOWOBSERVER_H

#include "WorkflowInstance.h"

namespace tividy {
class WorkflowInstance;

/**
 * @brief Observer pattern: abstract Observer.
 *
 * Implemented by anything that needs to react when a WorkflowInstance
 * changes, such as logging, progress display or notifications. The instance
 * (the Subject) knows only this interface, never the concrete observers.
 *
 * @par Pull model
 * update() passes the instance, not the changed data. The observer reads
 * what it needs from the instance. The instance does not decide what each
 * observer sees.
 *
 * @par Ownership
 * WorkflowInstance keeps a list of observers but does not own them
 * (aggregation). Whoever creates an observer deletes it, and must detach it
 * from every instance first. The instance never deletes an observer.
 *
 * @par Concrete observers
 * NotificationObserver, HistoryObserver and ProgressObserver.
 *
 * @see WorkflowInstance
 */

class WorkflowObserver {

public:
/**
 * @brief Called by the instance after it has changed.
 *
 * The change is already complete when this runs, so the observer sees the
 * new state.
 *
 * @param instance The instance that changed. Never null. Valid only for
 *        the duration of this call, so do not store the pointer.
 *
 * @note An observer may detach itself during update(). It must not delete
 *       the instance or modify the instance's state.
 * @note If an observer throws, the instance records the failure and still
 *       notifies the remaining observers. The state change is not undone.
 */
	virtual void update(WorkflowInstance* instance) = 0;
/*
*@brief Virtual destructor to allow for proper memory management 
*/
	virtual ~WorkflowObserver(){};
};

#endif // WORKFLOWOBSERVER_H
}

