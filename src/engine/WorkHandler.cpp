#include "WorkHandler.h"
#include <stdexcept>
namespace tividy {
void WorkHandler::handleRequest(WorkflowInstance* instance) {
    if (instance == nullptr) throw std::invalid_argument("WorkHandler: null workflow instance");
    if (successor != nullptr) successor->handleRequest(instance);
}
}
