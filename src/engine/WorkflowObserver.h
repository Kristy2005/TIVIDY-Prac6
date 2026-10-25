/**
 * @file WorkflowObserver.h
 * @brief Observer pattern: Subject (WorkflowSubject) and Observer (WorkflowObserver) roles.
 */
#ifndef TIVIDY_ENGINE_WORKFLOWOBSERVER_H
#define TIVIDY_ENGINE_WORKFLOWOBSERVER_H

#include <vector>
#include "engine/WorkflowEvent.h"

namespace tividy {

/** @brief Anything that wants to react to workflow events. */
class WorkflowObserver {
public:
    virtual ~WorkflowObserver();
    /** @brief Called once per event, in the order the events happen. */
    virtual void update(const WorkflowEvent& event) = 0;
};

/** @brief Keeps a list of observers and tells them about events. Observers are not owned. */
class WorkflowSubject {
public:
    virtual ~WorkflowSubject();
    /** @brief Subscribe @p o (ignored if NULL or already subscribed). */
    void attach(WorkflowObserver* o);
    /** @brief Unsubscribe @p o. */
    void detach(WorkflowObserver* o);
    /** @return Number of subscribers. */
    size_t observerCount() const { return observers_.size(); }

protected:
    /** @brief Deliver @p event to every observer in subscription order. */
    void notify(const WorkflowEvent& event);

private:
    std::vector<WorkflowObserver*> observers_;
};

} // namespace tividy

#endif
