#include "WorkflowMemento.h"
#include "WorkflowState.h"     // needed so delete can run the state's destructor

namespace tividy {

WorkflowMemento::WorkflowMemento()

    : state(nullptr) {

}

WorkflowMemento::~WorkflowMemento() { delete state; }

void WorkflowMemento::setState(WorkflowState* state) {
    this->state = state;
}

WorkflowState* WorkflowMemento::getState() const {
    return state;
}

}
