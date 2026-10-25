/**
 * @file WorkflowDefinition.h
 * @brief The reusable blueprint: work tree, roles, assignment strategy and escalation chain.
 */
#ifndef TIVIDY_ENGINE_WORKFLOWDEFINITION_H
#define TIVIDY_ENGINE_WORKFLOWDEFINITION_H

#include <memory>
#include <string>
#include <vector>
#include "engine/AssignmentStrategy.h"
#include "engine/Handler.h"
#include "engine/WorkflowStage.h"

namespace tividy {

/**
 * @brief Immutable once built (by a WorkflowBuilder). Many independent instances can be created
 * from one definition; each gets its own clone of the work tree (Prototype), while the
 * stateless strategy and escalation chain are shared read-only.
 */
class WorkflowDefinition {
public:
    ~WorkflowDefinition();

    const std::string& name() const { return name_; }
    const std::vector<std::string>& roles() const { return roles_; }
    bool hasRole(const std::string& role) const;
    /** @return The prototype tree (read-only). */
    const WorkflowComponent& prototype() const { return *root_; }
    /** @brief Prototype: a fresh, independent copy of the work tree for a new instance. Caller owns it. */
    WorkflowComponent* createWorkTree() const;
    const AssignmentStrategy& strategy() const { return *strategy_; }
    /** @return Head of the escalation chain, or NULL if the definition has none. */
    const Handler* escalationChain() const { return chain_.get(); }

private:
    friend class ConcreteWorkflowBuilder;
    explicit WorkflowDefinition(const std::string& name);
    WorkflowDefinition(const WorkflowDefinition&);
    WorkflowDefinition& operator=(const WorkflowDefinition&);

    std::string name_;
    std::vector<std::string> roles_;
    std::unique_ptr<WorkflowStage> root_;
    std::unique_ptr<AssignmentStrategy> strategy_;
    std::unique_ptr<Handler> chain_;
};

} // namespace tividy

#endif
