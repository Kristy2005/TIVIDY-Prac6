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
}

WorkflowState* WorkflowMemento::getState() const {
    return state;
}

} // namespace tividy
