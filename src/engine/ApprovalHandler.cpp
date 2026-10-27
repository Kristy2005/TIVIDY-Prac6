#include "ApprovalHandler.h"
#include <stdexcept>
namespace tividy {
void ApprovalHandler::handleRequest(WorkflowInstance* instance) {
    if (instance == nullptr) throw std::invalid_argument("ApprovalHandler: null workflow instance");
    if (successor != nullptr) successor->handleRequest(instance);
}
}
