#include "TIVIDYFacade.h"
#include "WorkItem.h"
#include "WorkflowInstance.h"

#include <stdexcept>
#include <vector>

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

void TIVIDYFacade::completeWork(
    WorkflowInstance* instance,
    WorkItem* item
) {
    if (instance == nullptr || item == nullptr) {
        throw std::invalid_argument(
            "completeWork: null argument"
        );
    }

    // Update progress for this instance only.
    instance->completeWorkItem(item);

    // Check whether every work item in this instance is complete.
    const std::vector<WorkItemProgress>& progress =
        instance->getWorkItemProgress();

    bool allCompleted = true;

    for (std::size_t i = 0; i < progress.size(); ++i) {
        if (!progress[i].completed) {
            allCompleted = false;
            break;
        }
    }

    // Resolve the workflow only when all its work items are complete.
    if (allCompleted && !progress.empty()) {
        instance->resolve();
    }
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
