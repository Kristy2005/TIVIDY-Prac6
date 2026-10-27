#include "EscalationHandler.h"
#include <stdexcept>
namespace tividy {
void EscalationHandler::handleRequest(WorkflowInstance* instance) {
    if (instance == nullptr) throw std::invalid_argument("EscalationHandler: null workflow instance");
    if (successor != nullptr) successor->handleRequest(instance);
    else throw std::logic_error("EscalationHandler: no successor configured for final decision");
}
}
