#include "ConcreteWorkflowBuilder.h"
#include "WorkflowDefinition.h"
#include "WorkflowStage.h"
#include "SubProcess.h"
#include "PriorityDecorator.h"
#include "Participant.h"
#include <set>
#include <stdexcept>

namespace tividy{

namespace {

bool isBlank(const std::string& text) {
    return text.find_first_not_of(" \t\r\n") == std::string::npos;
}

// True if @p item is @p target or depends on it, directly or through other items.
bool dependsOn(const WorkItem* item, const WorkItem* target, std::set<const WorkItem*>& visited) {
    if (item == target) {
        return true;
    }
    if (!visited.insert(item).second) {
        return false;
    }
    const std::vector<WorkItem*>& dependencies = item->getDependencies();
    for (std::size_t i = 0; i < dependencies.size(); ++i) {
        if (dependsOn(dependencies[i], target, visited)) {
            return true;
        }
    }
    return false;
}

bool replaceChild(WorkflowComponent* parent, WorkflowComponent* oldChild, WorkflowComponent* newChild) {
    WorkflowStage* stage = dynamic_cast<WorkflowStage*>(parent);
    if (stage != nullptr) {
        return stage->replace(oldChild, newChild);
    }
    SubProcess* subProcess = dynamic_cast<SubProcess*>(parent);
    if (subProcess != nullptr) {
        return subProcess->replace(oldChild, newChild);
    }
    return false;
}

} // namespace

  ConcreteWorkflowBuilder::ConcreteWorkflowBuilder()
      : definition(new WorkflowDefinition()), ownsDefinition(true), name() {
  }

  ConcreteWorkflowBuilder::ConcreteWorkflowBuilder(const std::string& definitionName)
      : definition(new WorkflowDefinition(definitionName)), ownsDefinition(true), name(definitionName) {
  }

  ConcreteWorkflowBuilder::ConcreteWorkflowBuilder(WorkflowDefinition* existing)
      : definition(existing), ownsDefinition(false), name() {
      if (existing == nullptr) {
          throw std::invalid_argument("ConcreteWorkflowBuilder: definition cannot be null");
      }
  }

  ConcreteWorkflowBuilder::~ConcreteWorkflowBuilder() {
      if (ownsDefinition) {
          delete definition;
      }
      definition = nullptr;
  }

  void ConcreteWorkflowBuilder::ensureDefinition() {
      if (definition == nullptr) {
          definition = new WorkflowDefinition(name);
          ownsDefinition = true;
          openSubProcesses.clear();
          parents.clear();
          decorators.clear();
      }
  }

  WorkflowComponent* ConcreteWorkflowBuilder::rootStage() {
      ensureDefinition();
      WorkflowStage* stage = dynamic_cast<WorkflowStage*>(definition->getRoot());
      if (stage == nullptr) {
          stage = new WorkflowStage();
          definition->setOwnedRoot(stage);
      }
      return stage;
  }

  WorkflowComponent* ConcreteWorkflowBuilder::currentComposite() {
      if (openSubProcesses.empty()) {
          return rootStage();
      }
      return openSubProcesses.back();
  }

  void ConcreteWorkflowBuilder::attach(WorkflowComponent* component) {
      WorkflowComponent* target = currentComposite();

      std::map<const WorkflowComponent*, WorkflowComponent*>::const_iterator known =
          parents.find(component);
      if (known != parents.end() && known->second != target) {
          throw std::invalid_argument(
              "ConcreteWorkflowBuilder: component already belongs to a different stage or sub-process"
          );
      }

      WorkflowStage* stage = dynamic_cast<WorkflowStage*>(target);
      if (stage != nullptr) {
          stage->add(component);
      } else {
          SubProcess* subProcess = dynamic_cast<SubProcess*>(target);
          if (subProcess == nullptr) {
              throw std::logic_error("ConcreteWorkflowBuilder: current node cannot hold children");
          }
          subProcess->add(component);
      }
      parents[component] = target;
  }

  void ConcreteWorkflowBuilder::ensureInTree(WorkItem* item) {
      if (parents.find(item) != parents.end() || decorators.find(item) != decorators.end()) {
          return;
      }
      attach(item);
  }

  void ConcreteWorkflowBuilder::addWorkItem(WorkItem* item) {
    if (!item) throw std::invalid_argument("addWorkItem: item cannot be null");
    ensureDefinition();

    // A prioritised item is already in the tree through its decorator.
    std::map<const WorkItem*, PriorityDecorator*>::const_iterator decorated = decorators.find(item);
    if (decorated != decorators.end()) {
        if (parents[decorated->second] != currentComposite()) {
            throw std::invalid_argument(
                "addWorkItem: item already belongs to a different stage or sub-process"
            );
        }
        return;
    }
    attach(item);
  }

  WorkItem* ConcreteWorkflowBuilder::createWorkItem(const std::string& itemName,
                                                    const std::string& requiredRole) {
    if (isBlank(itemName)) {
        throw std::invalid_argument("createWorkItem: work item name cannot be empty");
    }
    ensureDefinition();

    WorkItem* item = new WorkItem(itemName, requiredRole);
    definition->adoptComponent(item);   // owned from here on, even if attach() throws
    attach(item);
    return item;
  }

  void ConcreteWorkflowBuilder::addRole(Role<void>* role) {
      if (!role) throw std::invalid_argument("addRole: role cannot be null");
      ensureDefinition();
      definition->addRole(role, false);
  }

  Role<void>* ConcreteWorkflowBuilder::createRole(const std::string& roleName) {
      if (isBlank(roleName)) {
          throw std::invalid_argument("createRole: role name cannot be empty");
      }
      ensureDefinition();

      const std::vector<Role<void>*> existing = definition->getRoles();
      for (std::size_t i = 0; i < existing.size(); ++i) {
          if (existing[i]->getName() == roleName) {
              return existing[i];
          }
      }

      Role<void>* role = new Role<void>(roleName);
      try {
          definition->addRole(role, true);
      } catch (...) {
          delete role;
          throw;
      }
      return role;
  }

  void ConcreteWorkflowBuilder::addParticipant(Participant* participant) {
      if (!participant) throw std::invalid_argument("addParticipant: participant cannot be null");
      ensureDefinition();
      definition->addParticipant(participant, false);
  }

  Participant* ConcreteWorkflowBuilder::createParticipant(const std::string& participantName,
                                                          const std::string& role) {
      if (isBlank(participantName)) {
          throw std::invalid_argument("createParticipant: participant name cannot be empty");
      }
      ensureDefinition();
      if (definition->findParticipant(participantName) != nullptr) {
          throw std::invalid_argument(
              "createParticipant: a participant named '" + participantName + "' already exists"
          );
      }

      Participant* participant = new Participant(participantName);
      try {
          if (!isBlank(role)) {
              createRole(role);            // keep the definition's role list complete
              participant->addRole(role);
          }
          definition->addParticipant(participant, true);
      } catch (...) {
          delete participant;
          throw;
      }
      return participant;
  }

    void ConcreteWorkflowBuilder::addDependency(WorkItem* before,WorkItem* after)
    {
        if (!before || !after) {
            throw std::invalid_argument("addDependency: items cannot be null");
        }
        if (before == after) {
            throw std::invalid_argument("addDependency: an item cannot depend on itself");
        }

        // Reject cycles before changing anything: if 'before' already depends
        // on 'after', adding the opposite edge would make both wait forever.
        std::set<const WorkItem*> visited;
        if (dependsOn(before, after, visited)) {
            throw std::invalid_argument("addDependency: dependency would create a cycle");
        }

        ensureDefinition();
        ensureInTree(before);
        ensureInTree(after);

        after->addDependency(before);
    }

  void ConcreteWorkflowBuilder::setAssignmentStrategy(AssignmentStrategy* strategy) {
      ensureDefinition();
      definition->setAssignmentStrategy(strategy);
  }

  void ConcreteWorkflowBuilder::setEscalationHandler(Handler* handler) {
      ensureDefinition();
      definition->setEscalationHandler(handler);
  }

  void ConcreteWorkflowBuilder::addPriority(WorkItem* item, int priority) {
      if (!item) throw std::invalid_argument("addPriority: item cannot be null");
      if (priority < 0) throw std::invalid_argument("addPriority: priority cannot be negative");
      ensureDefinition();

      // The value lives on the item so clones made for instances carry it.
      item->setPriority(priority);

      std::map<const WorkItem*, PriorityDecorator*>::iterator existing = decorators.find(item);
      if (existing != decorators.end()) {
          existing->second->setPriority(priority);
          return;
      }

      ensureInTree(item);
      WorkflowComponent* parent = parents[item];

      PriorityDecorator* decorator = new PriorityDecorator(item, priority);
      definition->adoptComponent(decorator);

      if (!replaceChild(parent, item, decorator)) {
          throw std::logic_error("addPriority: item is no longer in its stage or sub-process");
      }
      decorators[item] = decorator;
      parents[decorator] = parent;
  }

  void ConcreteWorkflowBuilder::beginSubProcess() {
      ensureDefinition();
      SubProcess* subProcess = new SubProcess();
      definition->adoptComponent(subProcess);
      attach(subProcess);
      openSubProcesses.push_back(subProcess);
  }

  void ConcreteWorkflowBuilder::endSubProcess() {
      if (openSubProcesses.empty()) {
          throw std::logic_error("endSubProcess: no sub-process is open");
      }
      openSubProcesses.pop_back();
  }

  WorkflowDefinition* ConcreteWorkflowBuilder::getResult() {
      if (!openSubProcesses.empty()) {
          throw std::logic_error("getResult: a sub-process is still open");
      }
      if (!ownsDefinition) {
          return definition;      // configuring mode: still owned by someone else
      }

      WorkflowDefinition* result = this->definition;
      this->definition = nullptr;
      parents.clear();
      decorators.clear();
      return result;
  }
}
