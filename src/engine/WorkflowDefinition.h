#ifndef TIVIDY_WORKFLOW_DEFINITION_H
#define TIVIDY_WORKFLOW_DEFINITION_H

#include <string>
#include <vector>

#include "WorkflowComponent.h"
#include "AssignmentStrategy.h"
#include "Handler.h"

namespace tividy{
  class Participant;
  class WorkItem;

  /**
   * @brief Static workflow blueprint, including its configured assignment strategy.
   *
   * WorkflowDefinition keeps a non-owning pointer to the selected strategy.
   * The strategy object must outlive this definition. Root components and
   * escalation handlers are also non-owning pointers.
   */
  class WorkflowDefinition {
  public:
      /** @brief Construct an empty definition with no configured strategy. */
      WorkflowDefinition();

      /** @brief Set the root composite component. The pointer is non-owning. */
      void setRoot(WorkflowComponent* component);

      /**
       * @brief Configure or replace the strategy used for participant selection.
       * @param strategy Strategy to use, or nullptr to clear the configuration.
       */
      void setAssignmentStrategy(AssignmentStrategy* strategy);

      /** @brief Return the configured strategy, or nullptr if none is configured. */
      AssignmentStrategy* getAssignmentStrategy() const;

      /**
       * @brief Delegate participant selection to the configured strategy.
       * @param item Work item to route.
       * @param participants Candidate participants.
       * @return Selected participant, or nullptr if no strategy or candidate exists.
       */
      Participant* selectAssignee(WorkItem* item,
                                  std::vector<Participant*> participants) const;

      /** @brief Set the escalation handler. The pointer is non-owning. */
      void setEscalationHandler(Handler* handler);

      /** @brief Create a running instance from this definition. */
      WorkflowInstance* createInstance();

      /**
       * @brief Escalate a running instance to the configured handler.
       * @param instance Instance to escalate.
       * @throws std::logic_error if no handler is configured.
       */
        void handleEscalation(WorkflowInstance* instance);

  private:
      std::string name;
      WorkflowComponent* root;
      AssignmentStrategy* strategy;
      Handler* escalationHandler;
  };
}
#endif // TIVIDY_WORKFLOW_DEFINITION_H
