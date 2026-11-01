#include "TIVIDYFacade.h"
#include "WorkItem.h"
#include "WorkflowInstance.h"

#include <stdexcept>
#include <vector>

namespace tividy {

WorkflowDefinition* TIVIDYFacade::createWorkflow(const std::string& name) {
    WorkflowDefinition* definition = new WorkflowDefinition(name);
    this->definitions.push_back(definition);
    return definition;
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
    this->instances.push_back(instance);
    return instance;
}

void TIVIDYFacade::assignWork(WorkflowInstance* instance,WorkItem* item)
{
    if (instance == nullptr || item == nullptr) {
        throw std::invalid_argument("assignWork: instance and item cannot be null");
    }

    if (!instance->areDependenciesComplete(item)) {throw std::logic_error("assignWork: work item dependencies are not complete");
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

Participant* TIVIDYFacade::assignWork(WorkflowInstance* instance,WorkItem* item,std::vector<Participant*> participants)
{
    if (instance == nullptr || item == nullptr) {
        throw std::invalid_argument("assignWork: instance and item cannot be null");
    }

    if (!instance->areDependenciesComplete(item)) {
        return nullptr;
    }

    WorkflowDefinition* definition = instance->getDefinition();

    if (definition == nullptr) {
        throw std::logic_error("assignWork: workflow definition is not configured");
    }

    // Ask the definition's strategy to select a participant.
    Participant* selected = definition->selectAssignee(
        instance,
        item,
        participants
    );

    if (selected == nullptr) {
        return nullptr;
    }

    // Record the assignment against this specific instance.
    selected->assignWork(instance, item);

    // Execute the work item without marking it complete.
    item->execute();

    return selected;
}

void TIVIDYFacade::completeWork(
    WorkflowInstance* instance,
    WorkItem* item,
    Participant* participant) {

    if (instance == nullptr || item == nullptr ||
        participant == nullptr) {

        throw std::invalid_argument(
            "completeWork: null argument"
        );
    }

    // Complete the assignment and update participant workload.
    participant->completeWork(instance, item);

    // Check whether every work item in this instance is complete.
    const std::vector<WorkItemProgress>& progress =
        instance->getWorkItemProgress();

    bool allCompleted = !progress.empty();

    for (std::size_t i = 0; i < progress.size(); ++i) {
        if (!progress[i].completed) {
            allCompleted = false;
            break;
        }
    }

    if (allCompleted) {
        instance->resolve();
    }
}

std::string TIVIDYFacade::getHistory(WorkflowInstance* instance) {
    if (instance == nullptr) {
        throw std::invalid_argument(
            "getHistory: instance cannot be null"
        );
    }

    return instance->getHistory().describeEvents();
}

double TIVIDYFacade::getProgress(WorkflowInstance* instance){
    
    if (instance == nullptr) {
        throw std::invalid_argument(
            "getProgress: instance cannot be null"
        );
    }

    const std::vector<WorkItemProgress>& progress =
        instance->getWorkItemProgress();

    if (progress.empty()) {
        return 0.0;
    }

    int completedCount = 0;

    for (const auto& itemProgress : progress) {
        if (itemProgress.completed) {
            ++completedCount;
        }
    }

    return (static_cast<double>(completedCount) / progress.size()) * 100.0;
}

std::vector<WorkItem*> TIVIDYFacade::getIncompleteWorkItems(WorkflowInstance* instance) {
    if (instance == nullptr) {
        throw std::invalid_argument(
            "getIncompleteWorkItems: instance cannot be null"
        );
    }

    std::vector<WorkItem*> incompleteItems;
    const std::vector<WorkItemProgress>& progress = instance->getWorkItemProgress();

    for (const auto& itemProgress : progress) {
        if (!itemProgress.completed) {
            incompleteItems.push_back(itemProgress.item);
        }
    }

    return incompleteItems;
}
std::vector<WorkItem*> TIVIDYFacade::getAllWorkItems(WorkflowInstance* instance) {
    if (instance == nullptr) {
        throw std::invalid_argument(
            "getAllWorkItems: instance cannot be null"
        );
    }

    std::vector<WorkItem*> allItems;
    const std::vector<WorkItemProgress>& progress = instance->getWorkItemProgress();

    for (const auto& itemProgress : progress) {
        allItems.push_back(itemProgress.item);
    }

    return allItems;
}

std::vector<std::string> TIVIDYFacade::getParticipants(WorkflowInstance* instance) {
    if (instance == nullptr) {
        throw std::invalid_argument(
            "getParticipants: instance cannot be null"
        );
    }

    return instance->getParticipants();
    }

std::vector<WorkflowInstance*> TIVIDYFacade::getAllInstances() const {
    return instances;
}

std::vector<WorkflowDefinition*> TIVIDYFacade::getAllDefinitions() const {
    return definitions;     
}

std::vector<WorkflowInstance*> TIVIDYFacade::getInstancesForDefinition(WorkflowDefinition* definition) const {
    if (definition == nullptr) {
        throw std::invalid_argument(
            "getInstancesForDefinition: definition cannot be null"
        );
    }

    std::vector<WorkflowInstance*> result;

    for (WorkflowInstance* instance : instances) {
        if (instance != nullptr && instance->getDefinition() == definition) {
            result.push_back(instance);
        }
    }

    return result;
}

std::vector<Role<void>*> TIVIDYFacade::getRolesForDefinition(WorkflowDefinition* definition) const {
    if (definition == nullptr) {
        throw std::invalid_argument(
            "getRolesForDefinition: definition cannot be null"
        );
    }

    return definition->getRoles();
}

}
