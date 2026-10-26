#include "ConcreteWorkflowBuilder.h"
#include "WorkflowDefinition.h"

namespace tividy{
  ConcreteWorkflowBuilder::ConcreteWorkflowBuilder()
      : definition(new WorkflowDefinition()) {
  }

  ConcreteWorkflowBuilder::ConcreteWorkflowBuilder(const ConcreteWorkflowBuilder& other) : definition(nullptr) {
      if (other.definition != nullptr) {
          definition = new WorkflowDefinition(*other.definition);
      }
  }

  ConcreteWorkflowBuilder& ConcreteWorkflowBuilder::operator=(
      const ConcreteWorkflowBuilder& other) {
      if (this == &other) {
          return *this;
      }

      WorkflowDefinition* replacement = nullptr;
      if (other.definition != nullptr) {
          replacement = new WorkflowDefinition(*other.definition);
      }

      delete definition;
      definition = replacement;
      return *this;
  }

  ConcreteWorkflowBuilder::~ConcreteWorkflowBuilder() {
      delete definition;
  }

  void ConcreteWorkflowBuilder::addWorkItem(WorkItem* item) {
      (void)item;
      throw "Not yet implemented";
  }

  void ConcreteWorkflowBuilder::addRole(Role<void>* role) {
      (void)role;
      throw "Not yet implemented";
  }

  void ConcreteWorkflowBuilder::addDependency(WorkItem* before, WorkItem* after) {
      (void)before;
      (void)after;
      throw "Not yet implemented";
  }

  void ConcreteWorkflowBuilder::setAssignmentStrategy(AssignmentStrategy* strategy) {
      if (definition == nullptr) {
          definition = new WorkflowDefinition();
      }
      definition->setAssignmentStrategy(strategy);
  }

  void ConcreteWorkflowBuilder::setEscalationHandler(Handler* handler) {
      (void)handler;
      throw "Not yet implemented";
  }

  void ConcreteWorkflowBuilder::addPriority(WorkItem* item, int priority) {
      (void)item;
      (void)priority;
      throw "Not yet implemented";
  }

  WorkflowDefinition* ConcreteWorkflowBuilder::getResult() {
      WorkflowDefinition* result = definition;
      definition = nullptr;
      return result;
  }
}
