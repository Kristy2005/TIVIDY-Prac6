#include "engine/AssignmentStrategy.h"

namespace tividy {

AssignmentStrategy::~AssignmentStrategy() {}

bool AssignmentStrategy::eligible(const WorkflowComponent& item, const Participant& p) {
    return p.isAvailable() && p.role().name() == item.requiredRole();
}

Participant* RoleBasedAssignment::select(const WorkflowComponent& item, const std::vector<Participant*>& candidates) const {
    for (size_t i = 0; i < candidates.size(); ++i)
        if (eligible(item, *candidates[i])) return candidates[i];
    return 0;
}

Participant* WorkloadBasedAssignment::select(const WorkflowComponent& item, const std::vector<Participant*>& candidates) const {
    Participant* best = 0;
    for (size_t i = 0; i < candidates.size(); ++i) {
        if (!eligible(item, *candidates[i])) continue;
        if (!best || candidates[i]->workload() < best->workload()) best = candidates[i];
    }
    return best;
}

} // namespace tividy
