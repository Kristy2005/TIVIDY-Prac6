#ifndef PROGRESSOBSERVER_H
#define PROGRESSOBSERVER_H

#include "WorkflowObserver.h"
#include "WorkflowInstance.h"

namespace tividy {

/**
 * @brief Observer pattern: ConcreteObserver. Shows that a workflow changed.
 *
 * Prints "Workflow state changed." to the console whenever it is notified.
 * It makes the observer mechanism visible during a demonstration.
 *
 * @todo Report the actual state name and how much work is done. It cannot
 *       yet, because it keeps no data between notifications.
 *
 * @see WorkflowObserver
 */
class ProgressObserver : public WorkflowObserver {

public:
	/**
	 * @brief Prints a progress message.
	 *
	 * @param instance The instance that changed. Not stored.
	 */
	virtual void update(WorkflowInstance* instance) override;
};

} // namespace tividy

#endif // PROGRESSOBSERVER_H