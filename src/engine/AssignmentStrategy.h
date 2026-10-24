/**
 * @file AssignmentStrategy.h
 * @brief Strategy pattern: interchangeable rules for choosing who receives a work item.
 */
#ifndef TIVIDY_ENGINE_ASSIGNMENTSTRATEGY_H
#define TIVIDY_ENGINE_ASSIGNMENTSTRATEGY_H

#include <string>
#include <vector>
#include "engine/Participant.h"
#include "engine/WorkflowComponent.h"

namespace tividy {

/** @brief Chooses a participant for a work item. Set on the WorkflowDefinition by the Builder. */
class AssignmentStrategy {
public:
    virtual ~AssignmentStrategy();
    /**
     * @param item        The item that needs an owner (priority and role can be read from it).
     * @param candidates  Every participant of the workflow instance.
     * @return The chosen participant, or NULL if nobody is eligible.
     */
    virtual Participant* select(const WorkflowComponent& item, const std::vector<Participant*>& candidates) const = 0;
    /** @return Short name for display. */
    virtual std::string name() const = 0;

protected:
    /** @return true if @p p is available and holds the role @p item requires. */
    static bool eligible(const WorkflowComponent& item, const Participant& p);
};

/** @brief Picks the first eligible participant holding the required role. */
class RoleBasedAssignment : public AssignmentStrategy {
public:
    Participant* select(const WorkflowComponent& item, const std::vector<Participant*>& candidates) const;
    std::string name() const { return "RoleBased"; }
};

/** @brief Picks the eligible participant with the fewest current assignments (ties: first listed). */
class WorkloadBasedAssignment : public AssignmentStrategy {
public:
    Participant* select(const WorkflowComponent& item, const std::vector<Participant*>& candidates) const;
    std::string name() const { return "WorkloadBased"; }
};

} // namespace tividy

#endif
