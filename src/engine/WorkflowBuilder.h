/**
 * @file WorkflowBuilder.h
 * @brief Builder pattern: step-by-step construction of a WorkflowDefinition.
 */
#ifndef TIVIDY_ENGINE_WORKFLOWBUILDER_H
#define TIVIDY_ENGINE_WORKFLOWBUILDER_H

#include <memory>
#include <string>
#include <vector>
#include "engine/WorkflowDefinition.h"

namespace tividy {

/** @brief Plain description of a work item to be added by a builder. */
struct WorkItemSpec {
    std::string id;
    std::string name;
    std::string role;
    Priority priority;
    std::vector<std::string> dependsOn;

    WorkItemSpec(const std::string& id_, const std::string& name_, const std::string& role_,
                 Priority priority_ = Priority::Normal)
        : id(id_), name(name_), role(role_), priority(priority_) {}
    /** @brief Chainable: this item waits for @p itemId. */
    WorkItemSpec& after(const std::string& itemId) { dependsOn.push_back(itemId); return *this; }
};

/** @brief Abstract Builder: the steps every definition builder offers. */
class WorkflowBuilder {
public:
    virtual ~WorkflowBuilder();

    /** @brief Start a new definition called @p name, discarding any unfinished one. */
    virtual void reset(const std::string& name) = 0;
    virtual void addRole(const std::string& role) = 0;
    /** @brief Add a stage under @p parentId ("" = top level). */
    virtual void addStage(const std::string& id, const std::string& name, const std::string& parentId = "") = 0;
    /** @brief Add a nested sub-process under @p parentId. */
    virtual void addSubProcess(const std::string& id, const std::string& name, const std::string& parentId = "") = 0;
    /** @brief Add a work item under @p parentId; wrapped in a PriorityDecorator if its priority is not Normal. */
    virtual void addWorkItem(const std::string& parentId, const WorkItemSpec& spec) = 0;
    virtual void addDependency(const std::string& itemId, const std::string& dependsOnId) = 0;
    /** @brief Decision branch: @p targetId only runs if @p sourceId completes with @p outcome. */
    virtual void addRoutingRule(const std::string& sourceId, const std::string& outcome, const std::string& targetId) = 0;
    /** @brief Choose how work is assigned. Takes ownership. */
    virtual void setAssignmentStrategy(AssignmentStrategy* strategy) = 0;
    /** @brief Append a link to the escalation chain. Takes ownership. */
    virtual void addEscalationHandler(Handler* handler) = 0;
    /** @brief Validate and return the finished definition. Throws WorkflowError if it is invalid. */
    virtual std::shared_ptr<WorkflowDefinition> build() = 0;
};

/** @brief The standard builder. */
class ConcreteWorkflowBuilder : public WorkflowBuilder {
public:
    ConcreteWorkflowBuilder();
    explicit ConcreteWorkflowBuilder(const std::string& name);

    void reset(const std::string& name);
    void addRole(const std::string& role);
    void addStage(const std::string& id, const std::string& name, const std::string& parentId = "");
    void addSubProcess(const std::string& id, const std::string& name, const std::string& parentId = "");
    void addWorkItem(const std::string& parentId, const WorkItemSpec& spec);
    void addDependency(const std::string& itemId, const std::string& dependsOnId);
    void addRoutingRule(const std::string& sourceId, const std::string& outcome, const std::string& targetId);
    void setAssignmentStrategy(AssignmentStrategy* strategy);
    void addEscalationHandler(Handler* handler);
    std::shared_ptr<WorkflowDefinition> build();

private:
    WorkflowComponent* mustFind(const std::string& id, const char* what);
    WorkflowComponent* parentFor(const std::string& parentId);
    void ensureOpen() const;
    void validate();

    std::unique_ptr<WorkflowDefinition> def_;
};

/** @brief Director: ready-made construction recipes that drive any WorkflowBuilder. */
class WorkflowDirector {
public:
    /**
     * @brief A straight-line process: each item depends on the previous one.
     * Roles are registered automatically from the specs.
     */
    static std::shared_ptr<WorkflowDefinition> buildSequential(WorkflowBuilder& builder, const std::string& name,
                                                                const std::vector<WorkItemSpec>& steps);
};

} // namespace tividy

#endif
