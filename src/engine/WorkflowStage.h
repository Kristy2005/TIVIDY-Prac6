/**
 * @file WorkflowStage.h
 * @brief Composite node: a stage holding work items and nested stages / sub-processes.
 */
#ifndef TIVIDY_ENGINE_WORKFLOWSTAGE_H
#define TIVIDY_ENGINE_WORKFLOWSTAGE_H

#include "engine/WorkflowComponent.h"

namespace tividy {

/** @brief A group of components treated as one (Composite). Owns its children. */
class WorkflowStage : public WorkflowComponent {
public:
    WorkflowStage(const std::string& id, const std::string& name);
    ~WorkflowStage();

    std::string kind() const { return "Stage"; }
    WorkflowComponent* clone() const;
    bool isLeaf() const { return false; }
    void add(WorkflowComponent* child);
    WorkflowComponent* find(const std::string& id);
    void collectLeaves(std::vector<WorkflowComponent*>& out);
    /** @return Pending until work starts; Completed when every leaf is terminal. */
    WorkStatus status() const;

    /** @return The direct children, in order. */
    const std::vector<WorkflowComponent*>& children() const { return children_; }

protected:
    /** @brief Factory method so subclasses are cloned as their own type. */
    virtual WorkflowStage* createEmpty() const;

private:
    std::vector<WorkflowComponent*> children_;
};

/** @brief A stage that represents nested work (a sub-process inside a process). */
class SubProcess : public WorkflowStage {
public:
    SubProcess(const std::string& id, const std::string& name) : WorkflowStage(id, name) {}
    std::string kind() const { return "SubProcess"; }

protected:
    WorkflowStage* createEmpty() const { return new SubProcess(id(), name()); }
};

} // namespace tividy

#endif
