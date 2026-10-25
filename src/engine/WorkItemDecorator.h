/**
 * @file WorkItemDecorator.h
 * @brief Decorator pattern: add behaviour to a work item without changing WorkItem.
 */
#ifndef TIVIDY_ENGINE_WORKITEMDECORATOR_H
#define TIVIDY_ENGINE_WORKITEMDECORATOR_H

#include "engine/WorkflowComponent.h"

namespace tividy {

/**
 * @brief Base decorator: wraps one component and forwards the whole interface.
 *
 * The decorator has the same id as the wrapped item, so dependencies and
 * routing rules written against the id keep working. It owns the wrapped item.
 */
class WorkItemDecorator : public WorkflowComponent {
public:
    explicit WorkItemDecorator(WorkflowComponent* wrapped);
    ~WorkItemDecorator();

    std::string displayName() const { return inner_->displayName(); }
    std::string kind() const { return inner_->kind(); }
    bool isLeaf() const { return inner_->isLeaf(); }
    void collectLeaves(std::vector<WorkflowComponent*>& out);
    WorkStatus status() const { return inner_->status(); }

    Priority priority() const { return inner_->priority(); }
    const std::string& requiredRole() const { return inner_->requiredRole(); }
    const std::vector<std::string>& dependencies() const { return inner_->dependencies(); }
    bool hasCondition() const { return inner_->hasCondition(); }
    const std::string& conditionSource() const { return inner_->conditionSource(); }
    const std::string& conditionOutcome() const { return inner_->conditionOutcome(); }
    const std::string& assignee() const { return inner_->assignee(); }
    const std::string& outcome() const { return inner_->outcome(); }
    ItemState state() const { return inner_->state(); }
    void setState(const ItemState& s) { inner_->setState(s); }

    void addDependency(const std::string& itemId) { inner_->addDependency(itemId); }
    void setCondition(const std::string& src, const std::string& outcome) { inner_->setCondition(src, outcome); }
    void makeAvailable() { inner_->makeAvailable(); }
    void assign(const std::string& participant) { inner_->assign(participant); }
    void unassign() { inner_->unassign(); }
    void complete(const std::string& outcome) { inner_->complete(outcome); }
    void skip() { inner_->skip(); }
    void cancel() { inner_->cancel(); }

protected:
    WorkflowComponent* inner_;
};

/** @brief Marks a work item as High or Urgent so strategies and views can favour it. */
class PriorityDecorator : public WorkItemDecorator {
public:
    PriorityDecorator(WorkflowComponent* wrapped, Priority level);

    Priority priority() const;
    std::string displayName() const;
    WorkflowComponent* clone() const;

private:
    Priority level_;
};

} // namespace tividy

#endif
