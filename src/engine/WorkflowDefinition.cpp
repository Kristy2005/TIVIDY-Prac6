#include "WorkflowDefinition.h"
#include "Participant.h"
#include "WorkItem.h"

namespace tividy{
  WorkflowDefinition::WorkflowDefinition()
      : name(), root(nullptr), strategy(nullptr), escalationHandler(nullptr) {
  }

  void WorkflowDefinition::setRoot(WorkflowComponent* component) {
      root = component;
  }

  void WorkflowDefinition::setAssignmentStrategy(AssignmentStrategy* strategyValue) {
      strategy = strategyValue;
  }

  AssignmentStrategy* WorkflowDefinition::getAssignmentStrategy() const {
      return strategy;
  }

  Participant* WorkflowDefinition::selectAssignee(
      WorkItem* item, std::vector<Participant*> participants) const {
      if (strategy == nullptr) {
          return nullptr;
      }

      return strategy->assign(item, participants);
  }

  void WorkflowDefinition::setEscalationHandler(Handler* handler) {
      escalationHandler = handler;
  }

  WorkflowInstance* WorkflowDefinition::createInstance() {
      // Instance construction is outside the Strategy-pattern implementation.
      throw "Not yet implemented";
  }
}