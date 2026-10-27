#include "ConcreteWorkflowBuilder.h"
#include "WorkflowDefinition.h"
#include "WorkflowStage.h"
#include "PriorityDecorator.h"
#include <stdexcept>

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
      if (!item) throw std::invalid_argument("addWorkItem: item cannot be null");
      if (!definition) definition = new WorkflowDefinition();
      WorkflowStage* stage = dynamic_cast<WorkflowStage*>(definition->getRoot());
      if (!stage) { stage = new WorkflowStage(); definition->setRoot(stage); }
      stage->add(item);
  }

  void ConcreteWorkflowBuilder::addRole(Role<void>* role) {
      if (!role) throw std::invalid_argument("addRole: role cannot be null");
  }

  void ConcreteWorkflowBuilder::addDependency(WorkItem* before, WorkItem* after) {
      if (!before || !after) throw std::invalid_argument("addDependency: items cannot be null");
      // The current WorkflowComponent model has no dependency metadata; both items are added to the workflow.
      addWorkItem(before);
      addWorkItem(after);
  }

  void ConcreteWorkflowBuilder::setAssignmentStrategy(AssignmentStrategy* strategy) {
      if (definition == nullptr) {
          definition = new WorkflowDefinition();
      }
      definition->setAssignmentStrategy(strategy);
  }

  void ConcreteWorkflowBuilder::setEscalationHandler(Handler* handler) {
      if (!definition) definition = new WorkflowDefinition();
      definition->setEscalationHandler(handler);
  }

  void ConcreteWorkflowBuilder::addPriority(WorkItem* item, int priority) {
      if (!item) throw std::invalid_argument("addPriority: item cannot be null");
      if (priority < 0) throw std::invalid_argument("addPriority: priority cannot be negative");
      // PriorityDecorator is available for callers that need priority metadata.
      (void)priority;
  }

  WorkflowDefinition* ConcreteWorkflowBuilder::getResult() {
      WorkflowDefinition* result = definition;
      definition = nullptr;
      return result;
  }
}
