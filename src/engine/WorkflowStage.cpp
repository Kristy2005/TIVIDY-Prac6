#include "engine/WorkflowStage.h"

namespace tividy {

WorkflowStage::WorkflowStage(const std::string& id, const std::string& name) : WorkflowComponent(id, name) {}

WorkflowStage::~WorkflowStage() {
    for (size_t i = 0; i < children_.size(); ++i) delete children_[i];
}

WorkflowStage* WorkflowStage::createEmpty() const { return new WorkflowStage(id(), name()); }

WorkflowComponent* WorkflowStage::clone() const {
    WorkflowStage* copy = createEmpty();
    for (size_t i = 0; i < children_.size(); ++i) copy->add(children_[i]->clone());
    return copy;
}

void WorkflowStage::add(WorkflowComponent* child) {
    if (!child) throw WorkflowError("Cannot add a null component");
    children_.push_back(child);
}

WorkflowComponent* WorkflowStage::find(const std::string& wanted) {
    if (wanted == id()) return this;
    for (size_t i = 0; i < children_.size(); ++i) {
        WorkflowComponent* hit = children_[i]->find(wanted);
        if (hit) return hit;
    }
    return 0;
}

void WorkflowStage::collectLeaves(std::vector<WorkflowComponent*>& out) {
    for (size_t i = 0; i < children_.size(); ++i) children_[i]->collectLeaves(out);
}

WorkStatus WorkflowStage::status() const {
    std::vector<WorkflowComponent*> leaves;
    // collectLeaves is non-const because it hands out mutable leaves; status() only reads them.
    const_cast<WorkflowStage*>(this)->collectLeaves(leaves);
    if (leaves.empty()) return WorkStatus::Completed;
    bool allTerminal = true, anyAssigned = false, anyAvailable = false, anyCompleted = false;
    for (size_t i = 0; i < leaves.size(); ++i) {
        WorkStatus s = leaves[i]->status();
        if (!isTerminal(s)) allTerminal = false;
        if (s == WorkStatus::Assigned) anyAssigned = true;
        if (s == WorkStatus::Available) anyAvailable = true;
        if (s == WorkStatus::Completed) anyCompleted = true;
    }
    if (allTerminal) return WorkStatus::Completed;
    if (anyAssigned) return WorkStatus::Assigned;
    if (anyAvailable || anyCompleted) return WorkStatus::Available;
    return WorkStatus::Pending;
}

} // namespace tividy
