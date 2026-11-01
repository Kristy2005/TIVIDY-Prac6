#include "InteractiveWorkflowSession.h"

#include "../engine/ActiveState.h"
#include "../engine/InProgressState.h"
#include "../engine/Participant.h"
#include "../engine/ProgressObserver.h"
#include "../engine/ResolvedState.h"
#include "../engine/SubmittedState.h"
#include "../engine/ClosedState.h"
#include "../engine/TIVIDYFacade.h"
#include "../engine/WorkItem.h"
#include "../engine/WorkflowDefinition.h"
#include "../engine/WorkflowInstance.h"

#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace tividy {
namespace {

bool readLine(const std::string& prompt, std::string& value) {
    std::cout << prompt;
    if (!std::getline(std::cin, value)) {
        return false;
    }
    return true;
}

bool readInteger(const std::string& prompt, int minimum, int maximum, int& value) {
    while (true) {
        std::string line;
        if (!readLine(prompt, line)) {
            return false;
        }

        std::istringstream input(line);
        int parsed = 0;
        char trailing = '\0';
        if ((input >> parsed) && !(input >> trailing) &&
            parsed >= minimum && parsed <= maximum) {
            value = parsed;
            return true;
        }

        std::cout << "Invalid number. Please enter a value from " << minimum
                  << " to " << maximum << ".\n";
    }
}

std::string stateName(const WorkflowInstance* instance) {
    if (instance == nullptr) return "Unknown";
    WorkflowState* state = instance->getState();
    if (dynamic_cast<SubmittedState*>(state) != nullptr) return "Submitted";
    if (dynamic_cast<ActiveState*>(state) != nullptr) return "Active";
    if (dynamic_cast<InProgressState*>(state) != nullptr) return "In Progress";
    if (dynamic_cast<ResolvedState*>(state) != nullptr) return "Resolved";
    if (dynamic_cast<ClosedState*>(state) != nullptr) return "Closed";
    return "Unknown";
}

void displayInstances(
    const std::vector<WorkflowInstance*>& instances,
    WorkflowInstance* current,
    TIVIDYFacade& facade) {
    std::cout << "\nRunning instances:\n";
    for (std::size_t i = 0; i < instances.size(); ++i) {
        std::cout << "  " << (instances[i] == current ? "> " : "  ")
                  << (i + 1) << ". " << stateName(instances[i])
                  << " | progress " << facade.getProgress(instances[i]) << "%\n";
    }
}

void displayParticipants(const std::vector<Participant*>& participants) {
    if (participants.empty()) {
        std::cout << "  (no participants registered)\n";
        return;
    }

    for (std::size_t i = 0; i < participants.size(); ++i) {
        if (participants[i] == nullptr) continue;
        std::cout << "  " << (i + 1) << ". " << participants[i]->getName() << " [";
        const std::vector<std::string>& roles = participants[i]->getRoles();
        for (std::size_t r = 0; r < roles.size(); ++r) {
            if (r != 0) std::cout << ", ";
            std::cout << roles[r];
        }
        std::cout << "] | workload " << participants[i]->getWorkload() << "\n";
    }
}

void addScenarioParticipants(
    WorkflowInstance* instance,
    const std::vector<Participant*>& participantTemplates) {
    // Every instance receives its own staff objects. Names and roles are
    // copied from the scenario templates, but assignments/workloads stay local.
    for (std::size_t i = 0; i < participantTemplates.size(); ++i) {
        const Participant* source = participantTemplates[i];
        if (source == nullptr) continue;

        Participant* copy = new Participant(source->getName());
        const std::vector<std::string>& roles = source->getRoles();
        for (std::size_t role = 0; role < roles.size(); ++role) {
            copy->addRole(roles[role]);
        }

        try {
            instance->addParticipant(copy, true);
        } catch (...) {
            delete copy;
            throw;
        }
    }
}

std::vector<WorkItem*> activeWorkItems(TIVIDYFacade& facade, WorkflowInstance* instance) {
    return facade.getAllWorkItems(instance);
}

void displayWorkItems(TIVIDYFacade& facade, WorkflowInstance* instance) {
    const std::vector<WorkItemProgress>& progress = instance->getWorkItemProgress();
    if (progress.empty()) {
        std::cout << "  (no work items)\n";
        return;
    }

    std::cout << "\nWork items for this instance:\n";
    for (std::size_t i = 0; i < progress.size(); ++i) {
        const WorkItemProgress& entry = progress[i];
        std::cout << "  " << (i + 1) << ". " << entry.item->getName() << " | role: "
                  << (entry.item->getRequiredRole().empty()
                          ? "(any)" : entry.item->getRequiredRole())
                  << " | priority: " << entry.item->getPriority() << " | ";
        if (entry.cancelled) {
            std::cout << "CANCELLED";
        } else if (entry.completed) {
            std::cout << "COMPLETED";
        } else {
            Participant* assignee = facade.getAssignee(instance, entry.item);
            std::cout << (assignee == nullptr ? "OPEN" : "ASSIGNED");
            if (assignee != nullptr) {
                std::cout << " to " << assignee->getName();
            }
        }
        std::cout << "\n";
    }
}

bool chooseWorkItem(
    const std::vector<WorkItem*>& items,
    const std::string& prompt,
    WorkItem*& selected) {
    selected = nullptr;
    if (items.empty()) {
        std::cout << "There are no suitable work items for this action.\n";
        return false;
    }

    for (std::size_t i = 0; i < items.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << items[i]->getName() << "\n";
    }

    int choice = 0;
    if (!readInteger(prompt, 1, static_cast<int>(items.size()), choice)) {
        return false;
    }
    if (choice < 1 || choice > static_cast<int>(items.size())) {
        return true;
    }
    selected = items[static_cast<std::size_t>(choice - 1)];
    return true;
}

void addWorkItem(TIVIDYFacade& facade, WorkflowInstance* instance) {
    std::string name;
    std::string role;
    if (!readLine("New work item name: ", name) ||
        !readLine("Required role: ", role)) {
        return;
    }

    WorkItem* dependency = nullptr;
    const std::vector<WorkItem*> items = activeWorkItems(facade, instance);
    if (!items.empty()) {
        std::cout << "Optional dependency (0 = none):\n";
        for (std::size_t i = 0; i < items.size(); ++i) {
            std::cout << "  " << (i + 1) << ". " << items[i]->getName() << "\n";
        }
        int dependencyChoice = 0;
        if (!readInteger("Dependency number: ", 0, static_cast<int>(items.size()), dependencyChoice)) {
            return;
        }
        if (dependencyChoice > 0 && dependencyChoice <= static_cast<int>(items.size())) {
            dependency = items[static_cast<std::size_t>(dependencyChoice - 1)];
        }
    }

    WorkItem* added = facade.addWorkItemToInstance(instance, name, role, dependency);
    std::cout << "Added '" << added->getName() << "' to this instance only.\n";
}

void cancelWorkItem(TIVIDYFacade& facade, WorkflowInstance* instance) {
    // Include completed items in the choice list so an attempted cancellation
    // is rejected by the engine with its explicit validation message.
    const std::vector<WorkItem*> items = facade.getAllWorkItems(instance);
    WorkItem* selected = nullptr;
    if (!chooseWorkItem(items, "Work item to cancel (completed items are rejected): ", selected) ||
        selected == nullptr) {
        return;
    }
    facade.cancelWorkItem(instance, selected);
    std::cout << "Cancelled '" << selected->getName() << "'.\n";
}

void alterWorkItem(TIVIDYFacade& facade, WorkflowInstance* instance) {
    const std::vector<WorkItem*> items = facade.getIncompleteWorkItems(instance);
    WorkItem* selected = nullptr;
    if (!chooseWorkItem(items, "Work item to alter: ", selected) || selected == nullptr) {
        return;
    }

    std::cout << "  1. Reassign participant\n"
              << "  2. Change required role\n"
              << "  3. Change priority\n";
    int action = 0;
    if (!readInteger("Alteration: ", 1, 3, action)) return;
    if (action < 1 || action > 3) return;

    if (action == 1) {
        const std::vector<Participant*> participants = facade.getParticipantObjects(instance);
        if (participants.empty()) {
            std::cout << "No participants are registered. Add a participant first.\n";
            return;
        }
        displayParticipants(participants);
        int participantChoice = 0;
        if (!readInteger("Assign to participant: ", 1,
                         static_cast<int>(participants.size()), participantChoice)) {
            return;
        }
        if (participantChoice < 1 || participantChoice > static_cast<int>(participants.size())) {
            return;
        }
        Participant* assignee = participants[static_cast<std::size_t>(participantChoice - 1)];
        if (facade.getAssignee(instance, selected) == assignee) {
            std::cout << "'" << selected->getName() << "' is already assigned to "
                      << assignee->getName() << ".\n";
            return;
        }
        facade.reassignWorkItem(instance, selected, assignee);
        std::cout << "Assigned '" << selected->getName() << "' to "
                  << assignee->getName() << ".\n";
    } else if (action == 2) {
        std::string role;
        if (!readLine("New required role: ", role)) return;
        Participant* previousAssignee = facade.getAssignee(instance, selected);
        facade.changeWorkItemRole(instance, selected, role);
        std::cout << "Updated the required role for '" << selected->getName() << "'.\n";
        if (previousAssignee != nullptr && facade.getAssignee(instance, selected) == nullptr) {
            std::cout << "The previous assignee was released because they do not have the new role.\n";
        }
    } else {
        int priority = 0;
        if (!readInteger("New priority (0 or higher; larger is more urgent): ",
                         0, std::numeric_limits<int>::max(), priority)) {
            return;
        }
        if (priority < 0) return;
        facade.changeWorkItemPriority(instance, selected, priority);
        std::cout << "Updated priority for '" << selected->getName() << "'.\n";
    }
}

void addParticipant(TIVIDYFacade& facade, WorkflowInstance* instance) {
    std::string name;
    std::string role;
    if (!readLine("Participant name: ", name) ||
        !readLine("Participant role: ", role)) {
        return;
    }

    if (role.find_first_not_of(" \t\r\n") == std::string::npos) {
        std::cout << "A participant must have a role.\n";
        return;
    }

    Participant* participant = new Participant(name);
    participant->addRole(role);
    try {
        facade.addParticipantToInstance(instance, participant, true);
    } catch (...) {
        delete participant;
        throw;
    }
    std::cout << "Added '" << name << "' with role '" << role
              << "' to this instance only.\n";
}

void assignWork(TIVIDYFacade& facade, WorkflowInstance* instance) {
    const std::vector<WorkItem*> items = facade.getIncompleteWorkItems(instance);
    WorkItem* selected = nullptr;
    if (!chooseWorkItem(items, "Work item to assign: ", selected) || selected == nullptr) {
        return;
    }

    if (!instance->areDependenciesComplete(selected)) {
        std::cout << "Cannot assign this item yet: its dependencies are not complete.\n";
        return;
    }
    if (facade.getAssignee(instance, selected) != nullptr) {
        std::cout << "This item is already assigned. Use Alter -> Reassign participant.\n";
        return;
    }

    const std::vector<Participant*> participants = facade.getParticipantObjects(instance);
    Participant* assignee = facade.assignWork(instance, selected, participants);
    if (assignee == nullptr) {
        std::cout << "No registered participant has the required role '"
                  << selected->getRequiredRole() << "'.\n";
        return;
    }
    std::cout << "Assigned '" << selected->getName() << "' to "
              << assignee->getName() << ".\n";
}

void completeAssignedWork(TIVIDYFacade& facade, WorkflowInstance* instance) {
    const std::vector<WorkItem*> items = facade.getAssignedWorkItems(instance);
    WorkItem* selected = nullptr;
    if (!chooseWorkItem(items, "Assigned work item to complete: ", selected) || selected == nullptr) {
        return;
    }

    Participant* assignee = facade.getAssignee(instance, selected);
    facade.completeWork(instance, selected, assignee);
    std::cout << "Completed '" << selected->getName() << "'.\n";
}

void startAnotherInstance(
    TIVIDYFacade& facade,
    WorkflowDefinition* definition,
    const std::vector<Participant*>& initialParticipants,
    ProgressObserver& observer,
    std::vector<WorkflowInstance*>& instances) {
    WorkflowInstance* instance = facade.startWorkflow(definition);
    try {
        addScenarioParticipants(instance, initialParticipants);
        instance->attach(&observer);
        instance->startProcessing();
    } catch (...) {
        instance->detach(&observer);
        delete instance;
        throw;
    }

    instances.push_back(instance);
    std::cout << "Started instance " << instances.size()
              << " from the unchanged workflow definition.\n";
}

} // namespace

void runInteractiveWorkflowSession(
    TIVIDYFacade& facade,
    WorkflowDefinition* definition,
    WorkflowInstance* firstInstance,
    const std::vector<Participant*>& initialParticipants,
    const std::string& caseDescription) {
    if (definition == nullptr || firstInstance == nullptr) {
        throw std::invalid_argument("Interactive session requires a definition and first instance");
    }

    // Both industry scenarios read their one-character decision with >>.
    // Consume that line ending before the menu switches to getline().
    if (std::cin.good()) {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    std::vector<WorkflowInstance*> instances;
    instances.push_back(firstInstance);
    ProgressObserver observer;

    addScenarioParticipants(firstInstance, initialParticipants);
    firstInstance->attach(&observer);
    if (dynamic_cast<ActiveState*>(firstInstance->getState()) != nullptr) {
        firstInstance->startProcessing();
    }

    WorkflowInstance* current = firstInstance;
    bool running = true;
    std::cout << "\n============================================================\n"
              << " LIVE WORKFLOW CONTROLS - " << caseDescription << "\n"
              << " Changes affect the selected instance only.\n"
              << "============================================================\n";

    while (running && std::cin.good()) {
        displayInstances(instances, current, facade);
        displayWorkItems(facade, current);
        std::cout << "\nActions:\n"
                  << "  1. Select a running instance\n"
                  << "  2. Add work item\n"
                  << "  3. Cancel an incomplete work item\n"
                  << "  4. Alter work (reassign, role or priority)\n"
                  << "  5. Add participant\n"
                  << "  6. Start another instance of this definition\n"
                  << "  7. Assign available work\n"
                  << "  8. Complete assigned work\n"
                  << "  9. View this instance's audit history\n"
                  << "  0. Return to the main TIVIDY menu\n";

        int choice = -1;
        if (!readInteger("Select action: ", 0, 9, choice)) {
            break;
        }
        if (choice < 0 || choice > 9) {
            continue;
        }

        try {
            switch (choice) {
                case 0:
                    running = false;
                    break;
                case 1: {
                    int instanceChoice = 0;
                    if (!readInteger("Select instance number: ", 1,
                                     static_cast<int>(instances.size()), instanceChoice)) {
                        running = false;
                        break;
                    }
                    if (instanceChoice >= 1 && instanceChoice <= static_cast<int>(instances.size())) {
                        current = instances[static_cast<std::size_t>(instanceChoice - 1)];
                    }
                    break;
                }
                case 2:
                    addWorkItem(facade, current);
                    break;
                case 3:
                    cancelWorkItem(facade, current);
                    break;
                case 4:
                    alterWorkItem(facade, current);
                    break;
                case 5:
                    addParticipant(facade, current);
                    break;
                case 6:
                    startAnotherInstance(facade, definition, initialParticipants,
                                         observer, instances);
                    break;
                case 7:
                    assignWork(facade, current);
                    break;
                case 8:
                    completeAssignedWork(facade, current);
                    break;
                case 9:
                    std::cout << "\nAudit history for this instance:\n";
                    std::cout << facade.getHistory(current);
                    if (facade.getHistory(current).empty()) {
                        std::cout << "  (no events recorded)\n";
                    }
                    break;
            }
        } catch (const std::exception& error) {
            std::cout << "Change rejected: " << error.what() << "\n";
        }
    }

    std::cout << "\nSession summary:\n";
    for (std::size_t i = 0; i < instances.size(); ++i) {
        std::cout << "  Instance " << (i + 1) << ": " << stateName(instances[i])
                  << ", " << facade.getProgress(instances[i]) << "% complete\n";
        instances[i]->detach(&observer);
        delete instances[i];
    }
    std::cout << "Returning to the main TIVIDY menu.\n";
}

} // namespace tividy
