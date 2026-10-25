#include "engine/WorkflowBuilder.h"
#include <algorithm>
#include <map>
#include "engine/WorkItem.h"
#include "engine/WorkItemDecorator.h"

namespace tividy {

WorkflowBuilder::~WorkflowBuilder() {}

ConcreteWorkflowBuilder::ConcreteWorkflowBuilder() {}
ConcreteWorkflowBuilder::ConcreteWorkflowBuilder(const std::string& name) { reset(name); }

void ConcreteWorkflowBuilder::reset(const std::string& name) { def_.reset(new WorkflowDefinition(name)); }

void ConcreteWorkflowBuilder::ensureOpen() const {
    if (!def_) throw WorkflowError("No definition in progress: call reset(name) first");
}

WorkflowComponent* ConcreteWorkflowBuilder::mustFind(const std::string& id, const char* what) {
    ensureOpen();
    WorkflowComponent* c = def_->root_->find(id);
    if (!c) throw WorkflowError(std::string("Unknown ") + what + " '" + id + "'");
    return c;
}

WorkflowComponent* ConcreteWorkflowBuilder::parentFor(const std::string& parentId) {
    ensureOpen();
    if (parentId.empty()) return def_->root_.get();
    WorkflowComponent* p = mustFind(parentId, "parent stage");
    if (p->isLeaf()) throw WorkflowError("'" + parentId + "' is a work item and cannot contain other items");
    return p;
}

void ConcreteWorkflowBuilder::addRole(const std::string& role) {
    ensureOpen();
    if (!def_->hasRole(role)) def_->roles_.push_back(role);
}

void ConcreteWorkflowBuilder::addStage(const std::string& id, const std::string& name, const std::string& parentId) {
    WorkflowComponent* parent = parentFor(parentId);
    if (def_->root_->find(id)) throw WorkflowError("Duplicate id '" + id + "'");
    parent->add(new WorkflowStage(id, name));
}

void ConcreteWorkflowBuilder::addSubProcess(const std::string& id, const std::string& name, const std::string& parentId) {
    WorkflowComponent* parent = parentFor(parentId);
    if (def_->root_->find(id)) throw WorkflowError("Duplicate id '" + id + "'");
    parent->add(new SubProcess(id, name));
}

void ConcreteWorkflowBuilder::addWorkItem(const std::string& parentId, const WorkItemSpec& spec) {
    WorkflowComponent* parent = parentFor(parentId);
    if (def_->root_->find(spec.id)) throw WorkflowError("Duplicate id '" + spec.id + "'");
    WorkflowComponent* item = new WorkItem(spec.id, spec.name, spec.role);
    if (spec.priority != Priority::Normal) item = new PriorityDecorator(item, spec.priority);
    for (size_t i = 0; i < spec.dependsOn.size(); ++i) item->addDependency(spec.dependsOn[i]);
    parent->add(item);
}

void ConcreteWorkflowBuilder::addDependency(const std::string& itemId, const std::string& dependsOnId) {
    WorkflowComponent* item = mustFind(itemId, "work item");
    mustFind(dependsOnId, "work item");
    item->addDependency(dependsOnId);
}

void ConcreteWorkflowBuilder::addRoutingRule(const std::string& sourceId, const std::string& outcome,
                                             const std::string& targetId) {
    WorkflowComponent* target = mustFind(targetId, "work item");
    mustFind(sourceId, "work item");
    if (target->hasCondition()) throw WorkflowError("'" + targetId + "' already has a routing rule");
    target->setCondition(sourceId, outcome);
}

void ConcreteWorkflowBuilder::setAssignmentStrategy(AssignmentStrategy* strategy) {
    std::unique_ptr<AssignmentStrategy> owned(strategy);
    ensureOpen();
    if (!owned) throw WorkflowError("Strategy must not be null");
    def_->strategy_ = std::move(owned);
}

void ConcreteWorkflowBuilder::addEscalationHandler(Handler* handler) {
    std::unique_ptr<Handler> owned(handler);
    ensureOpen();
    if (!owned) throw WorkflowError("Handler must not be null");
    if (!def_->chain_) def_->chain_ = std::move(owned);
    else def_->chain_->append(owned.release());
}

void ConcreteWorkflowBuilder::validate() {
    std::vector<WorkflowComponent*> leaves;
    def_->root_->collectLeaves(leaves);
    if (leaves.empty()) throw WorkflowError("A workflow needs at least one work item");

    std::map<std::string, std::vector<std::string> > graph;
    for (size_t i = 0; i < leaves.size(); ++i) {
        WorkflowComponent* item = leaves[i];
        if (!item->requiredRole().empty() && !def_->hasRole(item->requiredRole()))
            throw WorkflowError("Item '" + item->id() + "' needs undeclared role '" + item->requiredRole() + "'");
        const std::vector<std::string>& deps = item->dependencies();
        for (size_t d = 0; d < deps.size(); ++d) {
            WorkflowComponent* dep = def_->root_->find(deps[d]);
            if (!dep || !dep->isLeaf())
                throw WorkflowError("Item '" + item->id() + "' depends on unknown work item '" + deps[d] + "'");
        }
        graph[item->id()] = deps;
    }

    // Cycle check: repeatedly remove items whose dependencies are all removed.
    std::map<std::string, std::vector<std::string> > remaining(graph);
    bool progress = true;
    while (!remaining.empty() && progress) {
        progress = false;
        for (std::map<std::string, std::vector<std::string> >::iterator it = remaining.begin(); it != remaining.end();) {
            bool free = true;
            for (size_t d = 0; d < it->second.size(); ++d)
                if (remaining.count(it->second[d])) { free = false; break; }
            if (free) { remaining.erase(it++); progress = true; }
            else ++it;
        }
    }
    if (!remaining.empty()) throw WorkflowError("Circular dependency involving '" + remaining.begin()->first + "'");
}

std::shared_ptr<WorkflowDefinition> ConcreteWorkflowBuilder::build() {
    ensureOpen();
    validate();
    if (!def_->strategy_) def_->strategy_.reset(new RoleBasedAssignment());
    std::shared_ptr<WorkflowDefinition> done(def_.release());
    return done;
}

std::shared_ptr<WorkflowDefinition> WorkflowDirector::buildSequential(WorkflowBuilder& builder, const std::string& name,
                                                                       const std::vector<WorkItemSpec>& steps) {
    builder.reset(name);
    builder.addStage("main", name);
    std::string previous;
    for (size_t i = 0; i < steps.size(); ++i) {
        builder.addRole(steps[i].role);
        WorkItemSpec spec(steps[i]);
        if (!previous.empty()) spec.after(previous);
        builder.addWorkItem("main", spec);
        previous = spec.id;
    }
    return builder.build();
}

} // namespace tividy
