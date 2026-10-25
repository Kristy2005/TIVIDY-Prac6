#include "engine/WorkItemDecorator.h"

namespace tividy {

WorkItemDecorator::WorkItemDecorator(WorkflowComponent* wrapped)
    : WorkflowComponent(wrapped ? wrapped->id() : "", wrapped ? wrapped->name() : ""), inner_(wrapped) {
    if (!wrapped) throw WorkflowError("A decorator needs a component to wrap");
}

WorkItemDecorator::~WorkItemDecorator() { delete inner_; }

void WorkItemDecorator::collectLeaves(std::vector<WorkflowComponent*>& out) {
    // The decorator itself is what the engine should talk to, so priority etc. stay visible.
    if (inner_->isLeaf()) out.push_back(this);
    else inner_->collectLeaves(out);
}

PriorityDecorator::PriorityDecorator(WorkflowComponent* wrapped, Priority level)
    : WorkItemDecorator(wrapped), level_(level) {}

Priority PriorityDecorator::priority() const {
    Priority base = inner_->priority();
    return static_cast<int>(level_) > static_cast<int>(base) ? level_ : base;
}

std::string PriorityDecorator::displayName() const {
    return std::string("[") + priorityName(priority()) + "] " + inner_->displayName();
}

WorkflowComponent* PriorityDecorator::clone() const { return new PriorityDecorator(inner_->clone(), level_); }

} // namespace tividy
