#include "engine/ProgressObserver.h"
#include <sstream>
#include "engine/WorkflowInstance.h"

namespace tividy {

ProgressObserver::ProgressObserver(const WorkflowInstance& instance)
    : instance_(instance), finished_(0), total_(0), last_(EventType::InstanceCreated) {}

void ProgressObserver::update(const WorkflowEvent& event) {
    last_ = event.type;
    total_ = instance_.totalItems();
    finished_ = instance_.finishedItems();
}

std::string ProgressObserver::summary() const {
    std::ostringstream os;
    os << finished_ << "/" << total_ << " (" << percent() << "%)";
    return os.str();
}

} // namespace tividy
