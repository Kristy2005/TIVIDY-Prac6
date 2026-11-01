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

    if (!instance->areDependenciesComplete(item)) {
        throw std::logic_error("assignWork: work item dependencies are not complete");
    }

    item->execute();
}

void TIVIDYFacade::completeWork(WorkflowInstance* instance, WorkItem* item) {
    if (instance == nullptr || item == nullptr) {
        throw std::invalid_argument("completeWork: null argument");
    }

    instance->completeWorkItem(item);

    const std::vector<WorkItemProgress>& progress = instance->getWorkItemProgress();
    bool allCompleted = true;

    for (std::size_t i = 0; i < progress.size(); ++i) {
        if (!progress[i].completed) {
            allCompleted = false;
            break;
        }
    }

    if (allCompleted && !progress.empty()) {
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

    if (!instance->areDependenciesComplete(item)) {
        return nullptr;
    }

    WorkflowDefinition* definition = instance->getDefinition();
    if (definition == nullptr) {
        throw std::logic_error("assignWork: workflow definition is not configured");
    }

    Participant* selected = definition->selectAssignee(instance, item, participants);
    if (selected == nullptr) {
        return nullptr;
    }

    selected->assignWork(instance, item);
    item->execute();

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

    participant->completeWork(instance, item);

    const std::vector<WorkItemProgress>& progress = instance->getWorkItemProgress();
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
        throw std::invalid_argument("getHistory: instance cannot be null");
    }

    return instance->getHistory().describeEvents();
}

double TIVIDYFacade::getProgress(WorkflowInstance* instance) {
    if (instance == nullptr) {
        throw std::invalid_argument("getProgress: instance cannot be null");
    }

    const std::vector<WorkItemProgress>& progress = instance->getWorkItemProgress();
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
        throw std::invalid_argument("getIncompleteWorkItems: instance cannot be null");
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
        throw std::invalid_argument("getAllWorkItems: instance cannot be null");
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
        throw std::invalid_argument("getParticipants: instance cannot be null");
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