#include "FinalDecisionHandler.h"
#include <stdexcept>
namespace tividy {
void FinalDecisionHandler::handleRequest(WorkflowInstance* instance) {
    if (instance == nullptr) throw std::invalid_argument("FinalDecisionHandler: null workflow instance");
    // The final handler accepts the escalation request; workflow state is unchanged
    // because the current state model has no EscalatedState.
}
}
