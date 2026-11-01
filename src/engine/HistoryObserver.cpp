#include "HistoryObserver.h"
#include "WorkflowInstance.h"
#include "WorkflowHistory.h"
#include "WorkflowState.h"

#include "SubmittedState.h"
#include "ActiveState.h"
#include "InProgressState.h"
#include "ResolvedState.h"
#include "ClosedState.h"

#include <stdexcept>
#include <string>

namespace tividy {

HistoryObserver::HistoryObserver(WorkflowHistory* history)
    : history(history){

    if (history == nullptr) {
        throw std::invalid_argument("HistoryObserver: history cannot be null");
    }
}

void HistoryObserver::update(WorkflowInstance* instance)
{
    if (instance == nullptr) {
        throw std::invalid_argument(
            "HistoryObserver: instance cannot be null"
        );
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

    history->recordEvent(
        "State changed to " + stateName
    );
}

} // namespace tividy