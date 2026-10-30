#include "TIVIDYFacade.h"
#include "WorkItem.h"
#include <stdexcept>

namespace tividy {

WorkflowDefinition* TIVIDYFacade::createWorkflow() {
    return new WorkflowDefinition();
}

WorkflowInstance* TIVIDYFacade::startWorkflow(WorkflowDefinition* definition) {
    if (definition == nullptr) {
        throw std::invalid_argument(
            "startWorkflow: definition cannot be null"
        );
    }

    WorkflowInstance* instance = definition->createInstance();
    instance->submit();
    instance->activate();

    return instance;
}

void TIVIDYFacade::assignWork(WorkflowInstance* instance, WorkItem* item) {
    if (instance == nullptr || item == nullptr) {
        throw std::invalid_argument(
            "assignWork: instance and item cannot be null"
        );
    }

    item->execute();
}

void TIVIDYFacade::completeWork(WorkflowInstance* instance, WorkItem* item) {
    if (instance == nullptr || item == nullptr) {
        throw std::invalid_argument(
            "completeWork: instance and item cannot be null"
        );
    }

    item->complete();

    // Do not resolve the workflow here.
    // A workflow may contain multiple work items, and the Facade
    // currently has no mechanism to verify that all have completed.
}

void TIVIDYFacade::escalateWork(WorkflowInstance* instance) {
    if (instance == nullptr) {
        throw std::invalid_argument(
            "escalateWork: instance cannot be null"
        );
    }

    instance->escalate();
}

void TIVIDYFacade::closeWorkflow(WorkflowInstance* instance) {
    if (instance == nullptr) {
        throw std::invalid_argument(
            "closeWorkflow: instance cannot be null"
        );
    }

    instance->close();
}

}
