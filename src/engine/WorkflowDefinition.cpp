#include "WorkflowDefinition.h"
#include "WorkflowInstance.h"
#include "Participant.h"
#include "WorkItem.h"

#include <algorithm>
#include <stdexcept>

namespace tividy {

WorkflowDefinition::WorkflowDefinition(const std::string& definitionName)
    : name(definitionName), root(nullptr), ownsRoot(false), strategy(nullptr),
      escalationHandler(nullptr), roles(), participants(), ownedComponents(),
      ownedRoles(), ownedParticipants() {}

WorkflowDefinition::WorkflowDefinition()
    : name(), root(nullptr), ownsRoot(false), strategy(nullptr),
      escalationHandler(nullptr), roles(), participants(), ownedComponents(),
      ownedRoles(), ownedParticipants() {}

const std::string& WorkflowDefinition::getName() const {
    return name;
}

void WorkflowDefinition::setName(const std::string& newName) {
    name = newName;
}

void WorkflowDefinition::setRoot(WorkflowComponent* component)
{
    if (ownsRoot && root != nullptr && root != component) {
        delete root;
    }

    root = component;
    ownsRoot = false;
}

WorkflowDefinition::~WorkflowDefinition(){
    if (ownsRoot) {
        delete root;
    }
    for (std::size_t i = 0; i < ownedComponents.size(); ++i) {
        delete ownedComponents[i];
    }
    for (std::size_t i = 0; i < ownedRoles.size(); ++i) {
        delete ownedRoles[i];
    }
    for (std::size_t i = 0; i < ownedParticipants.size(); ++i) {
        delete ownedParticipants[i];
    }
}

void WorkflowDefinition::adoptComponent(WorkflowComponent* component) {
    if (component == nullptr) {
        throw std::invalid_argument("adoptComponent: component cannot be null");
    }
    try {
        ownedComponents.push_back(component);
    } catch (...) {
        delete component;
        throw;
    }
}
void WorkflowDefinition::setOwnedRoot(WorkflowComponent* component)
{
    if (ownsRoot && root != nullptr && root != component) {
        delete root;
    }

    root = component;
    ownsRoot = true;
}

WorkflowComponent* WorkflowDefinition::getRoot() const {
    return root;
}

void WorkflowDefinition::setAssignmentStrategy(
AssignmentStrategy* strategyValue) {
strategy = strategyValue;
}

AssignmentStrategy* WorkflowDefinition::getAssignmentStrategy() const {
return strategy;
}

Participant* WorkflowDefinition::selectAssignee(WorkItem* item,std::vector<Participant*> participants) const {

    if (strategy == nullptr) {
        return nullptr;
    }

    return strategy->assign(item, participants);

}

void WorkflowDefinition::setEscalationHandler(Handler* handler) {
escalationHandler = handler;
}

WorkflowInstance* WorkflowDefinition::createInstance() {
return new WorkflowInstance(this);
}

void WorkflowDefinition::handleEscalation(WorkflowInstance* instance) {
    if (instance == nullptr) {
        throw std::invalid_argument(
            "WorkflowDefinition: instance cannot be null"
        );
    }

    if (escalationHandler == nullptr) {
        throw std::logic_error(
            "WorkflowDefinition: no escalation handler configured"
        );
    }

    escalationHandler->handleRequest(instance);
}

Participant* WorkflowDefinition::selectAssignee(WorkflowInstance* instance,WorkItem* item,std::vector<Participant*> participants) const {

    if (strategy == nullptr || instance == nullptr || item == nullptr) {
        return nullptr;
    }

    return strategy->assign(instance, item, participants);
}

WorkflowComponent* WorkflowDefinition::createWorkTree() const {
    if (root == nullptr) {
        return nullptr;
    }

    return root->clone();
}

void WorkflowDefinition::addRole(Role<void>* role, bool takeOwnership) {
    if (role == nullptr) {
        throw std::invalid_argument("addRole: role cannot be null");
    }

    for (std::size_t i = 0; i < roles.size(); ++i) {
        if (roles[i] == role) {
            if (takeOwnership &&
                std::find(ownedRoles.begin(), ownedRoles.end(), role) == ownedRoles.end()) {
                ownedRoles.push_back(role);
            }
            return;
        }
        if (!role->getName().empty() && roles[i]->getName() == role->getName()) {
            throw std::invalid_argument(
                "addRole: a role named '" + role->getName() + "' already exists in this definition"
            );
        }
    }

    roles.push_back(role);
    if (takeOwnership) {
        ownedRoles.push_back(role);
    }
}

void WorkflowDefinition::addParticipant(Participant* participant, bool takeOwnership) {
    if (participant == nullptr) {
        throw std::invalid_argument("addParticipant: participant cannot be null");
    }
    if (participant->getName().find_first_not_of(" \t\r\n") == std::string::npos) {
        throw std::invalid_argument("addParticipant: participant name cannot be empty");
    }

    for (std::size_t i = 0; i < participants.size(); ++i) {
        if (participants[i] == participant) {
            if (takeOwnership &&
                std::find(ownedParticipants.begin(), ownedParticipants.end(), participant)
                    == ownedParticipants.end()) {
                ownedParticipants.push_back(participant);
            }
            return;
        }
        if (participants[i]->getName() == participant->getName()) {
            throw std::invalid_argument(
                "addParticipant: a participant named '" + participant->getName() +
                "' already exists in this definition"
            );
        }
    }

    participants.push_back(participant);
    if (takeOwnership) {
        ownedParticipants.push_back(participant);
    }
}

std::vector<Participant*> WorkflowDefinition::getParticipants() const {
    return participants;
}

Participant* WorkflowDefinition::findParticipant(const std::string& participantName) const {
    for (std::size_t i = 0; i < participants.size(); ++i) {
        if (participants[i]->getName() == participantName) {
            return participants[i];
        }
    }
    return nullptr;
}

std::vector<Role<void>*> WorkflowDefinition::getRoles() const {
    return roles;   
}

}