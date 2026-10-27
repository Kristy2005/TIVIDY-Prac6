#include "Handler.h"
#include <stdexcept>

namespace tividy {

Handler::Handler() : successor(nullptr) {
}

Handler::~Handler() {
}

void Handler::setSuccessor(Handler* handler) {
    successor = handler;
}

void Handler::handleRequest(WorkflowInstance* instance) {
    if (instance == nullptr) {
        throw std::invalid_argument("Workflow instance cannot be null.");
    }

    if (successor != nullptr) {
        successor->handleRequest(instance);
    }
}

}