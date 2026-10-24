#include "engine/WorkflowHistory.h"
#include "engine/Types.h"

namespace tividy {

void WorkflowHistory::record(const WorkflowEvent& event) {
    WorkflowEvent copy(event);
    copy.sequence = static_cast<unsigned>(events_.size()) + 1;
    events_.push_back(copy);
}

std::vector<WorkflowEvent> WorkflowHistory::eventsOfType(EventType type) const {
    std::vector<WorkflowEvent> out;
    for (size_t i = 0; i < events_.size(); ++i)
        if (events_[i].type == type) out.push_back(events_[i]);
    return out;
}

std::string WorkflowHistory::report() const {
    std::string out;
    for (size_t i = 0; i < events_.size(); ++i) out += describe(events_[i]) + "\n";
    return out;
}

void WorkflowHistory::pushSnapshot(const WorkflowMemento& m) { snapshots_.push_back(m); }

WorkflowMemento WorkflowHistory::popSnapshot() {
    if (snapshots_.empty()) throw WorkflowError("There is no snapshot to restore");
    WorkflowMemento m = snapshots_.back();
    snapshots_.pop_back();
    return m;
}

} // namespace tividy
