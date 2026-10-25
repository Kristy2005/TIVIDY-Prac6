/**
 * @file WorkflowComponent.h
 * @brief Composite pattern: the common interface of work items, stages and sub-processes.
 *
 * Patterns: Composite (component), Prototype (clone) and the base for Decorator.
 * The interface is "transparent": leaf-only operations exist here and throw on
 * composites, so the engine can treat a single item and a whole stage uniformly.
 */
#ifndef TIVIDY_ENGINE_WORKFLOWCOMPONENT_H
#define TIVIDY_ENGINE_WORKFLOWCOMPONENT_H

#include <string>
#include <vector>
#include "engine/Types.h"

namespace tividy {

/** @brief Snapshot of one leaf's mutable data (used by Memento and Prototype). */
struct ItemState {
    WorkStatus status;
    std::string assignee;
    std::string outcome;
    ItemState() : status(WorkStatus::Pending) {}
};

/**
 * @brief Abstract node of the workflow tree.
 *
 * Ownership: composites own their children; the tree is cloned per workflow
 * instance (Prototype) so instances never share mutable work items.
 */
class WorkflowComponent {
public:
    WorkflowComponent(const std::string& id, const std::string& name);
    virtual ~WorkflowComponent();

    const std::string& id() const { return id_; }
    const std::string& name() const { return name_; }
    /** @return Name shown to users; decorators may add markers. */
    virtual std::string displayName() const { return name_; }
    /** @return A short kind label such as "WorkItem" or "Stage". */
    virtual std::string kind() const = 0;

    /** @brief Prototype: deep copy of this node and everything below it. */
    virtual WorkflowComponent* clone() const = 0;

    // ---- structure -------------------------------------------------------
    /** @return true for single work items (and decorators around them). */
    virtual bool isLeaf() const = 0;
    /** @brief Add a child; takes ownership. Throws WorkflowError on leaves. */
    virtual void add(WorkflowComponent* child);
    /** @return The node with the given id in this subtree, or NULL. */
    virtual WorkflowComponent* find(const std::string& id);
    /** @brief Read-only lookup (same search as the non-const version). */
    const WorkflowComponent* find(const std::string& id) const;
    /** @brief Append every leaf in this subtree, in definition order. */
    virtual void collectLeaves(std::vector<WorkflowComponent*>& out) = 0;
    /** @brief Number of leaves in this subtree whose status is @p s. */
    int countLeaves(WorkStatus s);

    // ---- status ----------------------------------------------------------
    /** @brief Leaf: its own status. Composite: derived from its leaves. */
    virtual WorkStatus status() const = 0;

    // ---- leaf-only data (default: throw WorkflowError) --------------------
    virtual Priority priority() const;
    virtual const std::string& requiredRole() const;
    virtual const std::vector<std::string>& dependencies() const;
    virtual bool hasCondition() const;
    virtual const std::string& conditionSource() const;
    virtual const std::string& conditionOutcome() const;
    virtual const std::string& assignee() const;
    virtual const std::string& outcome() const;
    virtual ItemState state() const;
    virtual void setState(const ItemState& s);

    // ---- leaf-only configuration (used by the Builder) --------------------
    virtual void addDependency(const std::string& itemId);
    virtual void setCondition(const std::string& sourceId, const std::string& outcome);

    // ---- leaf-only lifecycle (throws InvalidTransition when illegal) -------
    virtual void makeAvailable();
    virtual void assign(const std::string& participant);
    virtual void unassign();
    virtual void complete(const std::string& outcome);
    virtual void skip();
    virtual void cancel();

protected:
    /** @brief Throw the standard "leaf only" error for @p operation. */
    void leafOnly(const char* operation) const;

private:
    WorkflowComponent(const WorkflowComponent&);            // not copyable
    WorkflowComponent& operator=(const WorkflowComponent&);

    std::string id_;
    std::string name_;
};

} // namespace tividy

#endif
