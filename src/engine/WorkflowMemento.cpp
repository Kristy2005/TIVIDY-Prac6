#include "WorkflowMemento.h"

namespace tividy {

WorkflowMemento::WorkflowMemento()
    : state(nullptr) {
}

WorkflowMemento::~WorkflowMemento() {
    // The state is shared and is not owned by the memento.
}

void WorkflowMemento::setState(WorkflowState* state) {
    this->state = state;
    this->timestamp = std::time(nullptr); // Capture the current time when the state is set
}

WorkflowState* WorkflowMemento::getState() const {
    return state;
}

std::time_t WorkflowMemento::getTimestamp() const {
    return timestamp;
}

} // namespace tividy
