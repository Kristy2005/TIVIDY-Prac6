#include "WorkflowMemento.h"

namespace tividy {

WorkflowMemento::WorkflowMemento()

    : state(nullptr) {

}


void WorkflowMemento::setState(WorkflowState* state) {
    this->state = state;
}

WorkflowState* WorkflowMemento::getState() const {
    return state;
}

}
