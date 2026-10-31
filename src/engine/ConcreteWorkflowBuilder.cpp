#include "ConcreteWorkflowBuilder.h"
#include "WorkflowDefinition.h"
#include "WorkflowStage.h"
#include "PriorityDecorator.h"
#include <stdexcept>

namespace tividy{
  ConcreteWorkflowBuilder::ConcreteWorkflowBuilder()
      : definition(new WorkflowDefinition()) {
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

    void ConcreteWorkflowBuilder::addDependency(WorkItem* before,WorkItem* after)
    {
        if (!before || !after) {
            throw std::invalid_argument("addDependency: items cannot be null");
        }

        addWorkItem(before);
        addWorkItem(after);

        after->addDependency(before);
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
