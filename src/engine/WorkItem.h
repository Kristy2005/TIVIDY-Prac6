/**
 * @file WorkItem.h
 * @brief Composite leaf: one unit of work with its own lifecycle.
 */
#ifndef TIVIDY_ENGINE_WORKITEM_H
#define TIVIDY_ENGINE_WORKITEM_H

#include "engine/WorkflowComponent.h"

namespace tividy {

/**
 * @brief A single unit of work.
 *
 * Lifecycle: Pending -> Available -> Assigned -> Completed, or Pending ->
 * Skipped (branch not taken) or any non-terminal -> Cancelled. Illegal moves
 * throw InvalidTransition.
 *
 * An item may carry a routing condition ("only run if item X ended with
 * outcome Y"), which is how decision branches are expressed.
 */
class WorkItem : public WorkflowComponent {
public:
    WorkItem(const std::string& id, const std::string& name, const std::string& requiredRole);

    std::string kind() const { return "WorkItem"; }
    WorkflowComponent* clone() const;
    bool isLeaf() const { return true; }
    void collectLeaves(std::vector<WorkflowComponent*>& out);
    WorkStatus status() const { return state_.status; }

    const std::string& requiredRole() const { return role_; }
    const std::vector<std::string>& dependencies() const { return deps_; }
    bool hasCondition() const { return !condSource_.empty(); }
    const std::string& conditionSource() const { return condSource_; }
    const std::string& conditionOutcome() const { return condOutcome_; }
    const std::string& assignee() const { return state_.assignee; }
    const std::string& outcome() const { return state_.outcome; }
    ItemState state() const { return state_; }
    void setState(const ItemState& s) { state_ = s; }

    void addDependency(const std::string& itemId);
    void setCondition(const std::string& sourceId, const std::string& outcome);

    void makeAvailable();
    void assign(const std::string& participant);
    void unassign();
    void complete(const std::string& outcome);
    void skip();
    void cancel();

private:
    void require(WorkStatus expected, const char* action) const;

    std::string role_;
    std::vector<std::string> deps_;
    std::string condSource_;
    std::string condOutcome_;
    ItemState state_;
};

} // namespace tividy

#endif
