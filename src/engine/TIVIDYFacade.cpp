#include "TIVIDYFacade.h"
#include "WorkItem.h"
#include "WorkflowInstance.h"
#include "Participant.h"
#include "AssignmentStrategy.h"
#include "Handler.h"
#include "Role.h"
#include "ConcreteWorkflowBuilder.h"
#include "PriorityDecorator.h"

#include <stdexcept>
#include <vector>

namespace tividy {

WorkflowDefinition* TIVIDYFacade::createWorkflow(const std::string& name) {
    ConcreteWorkflowBuilder builder;
    // Constructing the workflow definition
    WorkflowDefinition* definition = new WorkflowDefinition(name);
    this->definitions.push_back(definition);
    return definition;
}

WorkflowDefinition* TIVIDYFacade::createWorkflow() {
    ConcreteWorkflowBuilder builder;
    // Constructing the workflow definition
    WorkflowDefinition* definition = new WorkflowDefinition();
    this->definitions.push_back(definition);
    return definition;
}

WorkflowInstance* TIVIDYFacade::startWorkflow(WorkflowDefinition* definition) {
    if (definition == nullptr) {
        throw std::invalid_argument("startWorkflow: definition cannot be null");
    }

    WorkflowInstance* instance = definition->createInstance();
    instance->submit();
    instance->activate();
    this->instances.push_back(instance);
    return instance;
}

void TIVIDYFacade::assignWork(WorkflowInstance* instance, WorkItem* item) {
    if (instance == nullptr || item == nullptr) {
        throw std::invalid_argument("assignWork: instance and item cannot be null");
    }

    WorkItem* actualItem = instance->getInstanceWorkItem(item);
    if (instance->isWorkItemCancelled(actualItem)) {
        throw std::logic_error("assignWork: cancelled work items cannot be assigned");
    }
    if (!instance->areDependenciesComplete(actualItem)) {
        throw std::logic_error("assignWork: work item dependencies are not complete");
    }

    actualItem->execute();
}

void TIVIDYFacade::completeWork(WorkflowInstance* instance, WorkItem* item) {
    if (instance == nullptr || item == nullptr) {
        throw std::invalid_argument("completeWork: null argument");
    }

    instance->completeWorkItem(item);

    if (instance->areAllWorkItemsComplete()) {
        instance->resolve();
    }
}

void TIVIDYFacade::escalateWork(WorkflowInstance* instance) {
    if (instance == nullptr) {
        throw std::invalid_argument("escalateWork: instance cannot be null");
    }

    instance->escalate();
}

void TIVIDYFacade::closeWorkflow(WorkflowInstance* instance) {
    if (instance == nullptr) {
        throw std::invalid_argument("closeWorkflow: instance cannot be null");
    }

    instance->close();
}

Participant* TIVIDYFacade::assignWork(
    WorkflowInstance* instance,
    WorkItem* item,
    std::vector<Participant*> participants
) {
    if (instance == nullptr || item == nullptr) {
        throw std::invalid_argument("assignWork: instance and item cannot be null");
    }

    WorkItem* actualItem = instance->getInstanceWorkItem(item);
    if (instance->isWorkItemCancelled(actualItem) ||
        instance->isWorkItemComplete(actualItem)) {
        return nullptr;
    }
    if (!instance->areDependenciesComplete(actualItem)) {
        return nullptr;
    }

    WorkflowDefinition* definition = instance->getDefinition();
    if (definition == nullptr) {
        throw std::logic_error("assignWork: workflow definition is not configured");
    }

    Participant* selected = definition->selectAssignee(instance, actualItem, participants);
    if (selected == nullptr) {
        return nullptr;
    }

    instance->assignParticipant(actualItem, selected);
    actualItem->execute();

    return selected;
}

void TIVIDYFacade::completeWork(
    WorkflowInstance* instance,
    WorkItem* item,
    Participant* participant
) {
    if (instance == nullptr || item == nullptr || participant == nullptr) {
        throw std::invalid_argument("completeWork: null argument");
    }

    WorkItem* actualItem = instance->getInstanceWorkItem(item);
    if (instance->getAssignee(actualItem) != participant) {
        throw std::logic_error(
            "completeWork: participant is not assigned to this work item in this instance"
        );
    }

    participant->completeWork(instance, actualItem);

    if (instance->areAllWorkItemsComplete()) {
        instance->resolve();
    }
}

std::string TIVIDYFacade::getHistory(WorkflowInstance* instance) {
    if (instance == nullptr) {
        throw std::invalid_argument("getHistory: instance cannot be null");
    }

    return instance->getHistory().describeEvents();
}

double TIVIDYFacade::getProgress(WorkflowInstance* instance) {
    if (instance == nullptr) {
        throw std::invalid_argument("getProgress: instance cannot be null");
    }

    const std::vector<WorkItemProgress>& progress = instance->getWorkItemProgress();
    std::size_t activeCount = 0;
    std::size_t completedCount = 0;
    for (std::size_t i = 0; i < progress.size(); ++i) {
        if (progress[i].cancelled) {
            continue;
        }
        ++activeCount;
        if (progress[i].completed) {
            ++completedCount;
        }
    }

    if (activeCount == 0) {
        return 0.0;
    }
    return (static_cast<double>(completedCount) / activeCount) * 100.0;
}

std::vector<WorkItem*> TIVIDYFacade::getIncompleteWorkItems(WorkflowInstance* instance) {
    if (instance == nullptr) {
        throw std::invalid_argument("getIncompleteWorkItems: instance cannot be null");
    }

    std::vector<WorkItem*> incompleteItems;
    const std::vector<WorkItemProgress>& progress = instance->getWorkItemProgress();

    for (const auto& itemProgress : progress) {
        if (!itemProgress.completed && !itemProgress.cancelled) {
            incompleteItems.push_back(itemProgress.item);
        }
    }

    return incompleteItems;
}

std::vector<WorkItem*> TIVIDYFacade::getAllWorkItems(WorkflowInstance* instance) {
    if (instance == nullptr) {
        throw std::invalid_argument("getAllWorkItems: instance cannot be null");
    }

    std::vector<WorkItem*> allItems;
    const std::vector<WorkItemProgress>& progress = instance->getWorkItemProgress();

    for (std::size_t i = 0; i < progress.size(); ++i) {
        if (!progress[i].cancelled) {
            allItems.push_back(progress[i].item);
        }
    }

    return allItems;
}

std::vector<std::string> TIVIDYFacade::getParticipants(WorkflowInstance* instance) {
    if (instance == nullptr) {
        throw std::invalid_argument("getParticipants: instance cannot be null");
    }

    return instance->getParticipants();
}

WorkItem* TIVIDYFacade::addWorkItemToInstance(
    WorkflowInstance* instance,
    const std::string& name,
    const std::string& requiredRole,
    WorkItem* dependency) {
    if (instance == nullptr) {
        throw std::invalid_argument("addWorkItemToInstance: instance cannot be null");
    }
    return instance->addWorkItem(name, requiredRole, dependency);
}

void TIVIDYFacade::cancelWorkItem(WorkflowInstance* instance, WorkItem* item) {
    if (instance == nullptr || item == nullptr) {
        throw std::invalid_argument("cancelWorkItem: instance and item cannot be null");
    }
    instance->cancelWorkItem(item);
    if (instance->areAllWorkItemsComplete()) {
        instance->resolve();
    }
}

void TIVIDYFacade::reassignWorkItem(
    WorkflowInstance* instance,
    WorkItem* item,
    Participant* participant) {
    if (instance == nullptr || item == nullptr || participant == nullptr) {
        throw std::invalid_argument("reassignWorkItem: arguments cannot be null");
    }
    instance->assignParticipant(item, participant);
}

void TIVIDYFacade::changeWorkItemRole(
    WorkflowInstance* instance,
    WorkItem* item,
    const std::string& requiredRole) {
    if (instance == nullptr || item == nullptr) {
        throw std::invalid_argument("changeWorkItemRole: instance and item cannot be null");
    }
    instance->changeWorkItemRole(item, requiredRole);
}

void TIVIDYFacade::changeWorkItemPriority(
    WorkflowInstance* instance,
    WorkItem* item,
    int priority) {
    if (instance == nullptr || item == nullptr) {
        throw std::invalid_argument("changeWorkItemPriority: instance and item cannot be null");
    }
    instance->changeWorkItemPriority(item, priority);
}

void TIVIDYFacade::addParticipantToInstance(
    WorkflowInstance* instance,
    Participant* participant,
    bool takeOwnership) {
    if (instance == nullptr || participant == nullptr) {
        throw std::invalid_argument("addParticipantToInstance: instance and participant cannot be null");
    }
    instance->addParticipant(participant, takeOwnership);
}

Participant* TIVIDYFacade::getAssignee(
    WorkflowInstance* instance,
    WorkItem* item) const {
    if (instance == nullptr || item == nullptr) {
        throw std::invalid_argument("getAssignee: instance and item cannot be null");
    }
    return instance->getAssignee(item);
}

std::vector<Participant*> TIVIDYFacade::getParticipantObjects(
    WorkflowInstance* instance) const {
    if (instance == nullptr) {
        throw std::invalid_argument("getParticipantObjects: instance cannot be null");
    }
    return instance->getParticipantPointers();
}

std::vector<WorkItem*> TIVIDYFacade::getAssignedWorkItems(
    WorkflowInstance* instance) const {
    if (instance == nullptr) {
        throw std::invalid_argument("getAssignedWorkItems: instance cannot be null");
    }

    std::vector<WorkItem*> assignedItems;
    const std::vector<WorkItemProgress>& progress = instance->getWorkItemProgress();
    for (std::size_t i = 0; i < progress.size(); ++i) {
        if (!progress[i].completed && !progress[i].cancelled &&
            instance->getAssignee(progress[i].item) != nullptr) {
            assignedItems.push_back(progress[i].item);
        }
    }
    return assignedItems;
}

std::vector<WorkflowInstance*> TIVIDYFacade::getAllInstances() const {
    return instances;
}

std::vector<WorkflowDefinition*> TIVIDYFacade::getAllDefinitions() const {
    return definitions;     
}

std::vector<WorkflowInstance*> TIVIDYFacade::getInstancesForDefinition(WorkflowDefinition* definition) const {
    if (definition == nullptr) {
        throw std::invalid_argument("getInstancesForDefinition: definition cannot be null");
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
        throw std::invalid_argument("getRolesForDefinition: definition cannot be null");
    }

    return definition->getRoles();
}

void TIVIDYFacade::addRoleToDefinition(WorkflowDefinition* definition, Role<void>* role) {
    if (definition == nullptr || role == nullptr) {
        throw std::invalid_argument("addRoleToDefinition: definition and role cannot be null");
    }
    ConcreteWorkflowBuilder builder;
    definition->addRole(role);
}

// void TIVIDYFacade::addParticipantToDefinition(WorkflowDefinition* definition, Participant* participant) {
//     if (definition == nullptr || participant == nullptr) {
//         throw std::invalid_argument("addParticipantToDefinition: definition and participant cannot be null");
//     }
//     // ConcreteWorkflowBuilder builder;
//     this->builder->addParticipant(participant);
// }

void TIVIDYFacade::configureAssignmentStrategy(WorkflowDefinition* definition, AssignmentStrategy* strategy) {
    if (definition == nullptr || strategy == nullptr) {
        throw std::invalid_argument("configureAssignmentStrategy: definition and strategy cannot be null");
    }
    ConcreteWorkflowBuilder builder;
    definition->setAssignmentStrategy(strategy);
}

void TIVIDYFacade::configureEscalationHandler(WorkflowDefinition* definition, Handler* handler) {
    if (definition == nullptr || handler == nullptr) {
        throw std::invalid_argument("configureEscalationHandler: definition and handler cannot be null");
    }
    ConcreteWorkflowBuilder builder;
    definition->setEscalationHandler(handler);
}

void TIVIDYFacade::addWorkItemToDefinition(WorkflowDefinition* definition, WorkItem* item) {
    if (definition == nullptr || item == nullptr) {
        throw std::invalid_argument("addWorkItemToDefinition: definition and item cannot be null");
    }
    // ConcreteWorkflowBuilder builder;
    this->builder->addWorkItem(item);
}

void TIVIDYFacade::addDependencyToWorkItem(WorkflowDefinition* definition, WorkItem* item, WorkItem* dependency) {
    if (definition == nullptr || item == nullptr || dependency == nullptr) {
        throw std::invalid_argument("addDependencyToWorkItem: arguments cannot be null");
    }
    item->addDependency(dependency);
}

void TIVIDYFacade::addPriorityToWorkItem(WorkflowDefinition* definition, WorkItem* item, int priority) {
    if (definition == nullptr || item == nullptr) {
        throw std::invalid_argument("addPriorityToWorkItem: definition and item cannot be null");
    }
    this->builder->addPriority(item, priority);
}

} // namespace tividy