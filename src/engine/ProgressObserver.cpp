#include "ProgressObserver.h"
#include <iostream>
#include <stdexcept>
namespace tividy {
void ProgressObserver::update(WorkflowInstance* instance) {
    if (!instance) throw std::invalid_argument("ProgressObserver: null instance");
    std::cout << "Workflow state changed." << std::endl;
}
}
