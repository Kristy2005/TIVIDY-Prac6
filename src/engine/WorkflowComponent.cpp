#include "engine/WorkflowComponent.h"

namespace tividy {

namespace {
const std::string kEmpty;
const std::vector<std::string> kNoDeps;
}

WorkflowComponent::WorkflowComponent(const std::string& id, const std::string& name)
    : id_(id), name_(name) {}

WorkflowComponent::~WorkflowComponent() {}

void WorkflowComponent::leafOnly(const char* operation) const {
    throw WorkflowError(std::string("'") + operation + "' is only valid on a single work item, not on '" + id_ + "'");
}

void WorkflowComponent::add(WorkflowComponent* child) {
    delete child;  // we took ownership, so don't leak when refusing
    leafOnly("add child");
}

WorkflowComponent* WorkflowComponent::find(const std::string& id) {
    return id == id_ ? this : 0;
}

const WorkflowComponent* WorkflowComponent::find(const std::string& id) const {
    return const_cast<WorkflowComponent*>(this)->find(id);
}

int WorkflowComponent::countLeaves(WorkStatus s) {
    std::vector<WorkflowComponent*> leaves;
    collectLeaves(leaves);
    int n = 0;
    for (size_t i = 0; i < leaves.size(); ++i)
        if (leaves[i]->status() == s) ++n;
    return n;
}

Priority WorkflowComponent::priority() const { return Priority::Normal; }
const std::string& WorkflowComponent::requiredRole() const { return kEmpty; }
const std::vector<std::string>& WorkflowComponent::dependencies() const { return kNoDeps; }
bool WorkflowComponent::hasCondition() const { return false; }
const std::string& WorkflowComponent::conditionSource() const { return kEmpty; }
const std::string& WorkflowComponent::conditionOutcome() const { return kEmpty; }
const std::string& WorkflowComponent::assignee() const { return kEmpty; }
const std::string& WorkflowComponent::outcome() const { return kEmpty; }
ItemState WorkflowComponent::state() const { leafOnly("state"); return ItemState(); }
void WorkflowComponent::setState(const ItemState&) { leafOnly("setState"); }
void WorkflowComponent::addDependency(const std::string&) { leafOnly("addDependency"); }
void WorkflowComponent::setCondition(const std::string&, const std::string&) { leafOnly("setCondition"); }
void WorkflowComponent::makeAvailable() { leafOnly("makeAvailable"); }
void WorkflowComponent::assign(const std::string&) { leafOnly("assign"); }
void WorkflowComponent::unassign() { leafOnly("unassign"); }
void WorkflowComponent::complete(const std::string&) { leafOnly("complete"); }
void WorkflowComponent::skip() { leafOnly("skip"); }
void WorkflowComponent::cancel() { leafOnly("cancel"); }

} // namespace tividy
