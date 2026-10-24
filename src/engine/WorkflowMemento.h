/**
 * @file WorkflowMemento.h
 * @brief Memento pattern: an opaque snapshot of a WorkflowInstance.
 */
#ifndef TIVIDY_ENGINE_WORKFLOWMEMENTO_H
#define TIVIDY_ENGINE_WORKFLOWMEMENTO_H

#include <map>
#include <string>
#include "engine/WorkflowComponent.h"

namespace tividy {

class WorkflowInstance;

/**
 * @brief Everything needed to roll an instance back (state, item progress, workloads,
 * escalation data, instance data). Only WorkflowInstance can create or read one, so the
 * instance's internals stay hidden from Commands and the history.
 *
 * The audit log is deliberately NOT part of the memento: history is append-only.
 */
class WorkflowMemento {
public:
    /** @return Name of the state the instance was in (for display only). */
    const std::string& stateName() const { return stateName_; }

private:
    friend class WorkflowInstance;
    WorkflowMemento() : escalationCount_(0) {}

    std::string stateName_;
    std::map<std::string, ItemState> items_;
    std::map<std::string, int> workloads_;
    std::map<std::string, std::string> data_;
    int escalationCount_;
    std::string escalatedTo_;
    std::string escalationReason_;
};

} // namespace tividy

#endif
