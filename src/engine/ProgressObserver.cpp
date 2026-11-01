#include "ProgressObserver.h"
#include "WorkflowInstance.h"
#include "WorkflowState.h"

#include "SubmittedState.h"
#include "ActiveState.h"
#include "InProgressState.h"
#include "ResolvedState.h"
#include "ClosedState.h"

#include <iostream>
#include <stdexcept>
#include <string>

namespace tividy {

void ProgressObserver::update(WorkflowInstance* instance)
{
    if (instance == nullptr) {
        throw std::invalid_argument("ProgressObserver: instance cannot be null");
    }

    WorkflowState* state = instance->getState();

    std::string stateName = "Unknown";

    if (dynamic_cast<SubmittedState*>(state) != nullptr) {
        stateName = "Submitted";
    }
    else if (dynamic_cast<ActiveState*>(state) != nullptr) {
        stateName = "Active";
    }
    else if (dynamic_cast<InProgressState*>(state) != nullptr) {
        stateName = "In Progress";
    }
    else if (dynamic_cast<ResolvedState*>(state) != nullptr) {
        stateName = "Resolved";
    }
    else if (dynamic_cast<ClosedState*>(state) != nullptr) {
        stateName = "Closed";
    }

    std::cout<< "Workflow state changed to "<< stateName<< "."<< std::endl;
}

} // namespace tividy