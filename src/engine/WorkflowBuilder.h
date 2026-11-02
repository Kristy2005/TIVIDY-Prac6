#ifndef TIVIDY_WORKFLOW_BUILDER_H
#define TIVIDY_WORKFLOW_BUILDER_H

#include <string>

#include "WorkItem.h"
#include "Role.h"
#include "Participant.h"
#include "AssignmentStrategy.h"
#include "Handler.h"

namespace tividy{
  class WorkflowDefinition;

  /**
   * @brief Interface for step-by-step construction of workflow definitions.
   *
   * Builder pattern: Builder. A client (or a WorkflowDirector) calls these
   * steps in any order and finally collects the product with getResult().
   *
   * Two kinds of step exist for work items, roles and participants:
   * - add...() registers an object the caller already created and keeps
   *   owning; the caller must keep it alive as long as the definition.
   * - create...() makes the object and hands its ownership to the definition,
   *   so nothing has to be deleted by the caller.
   */
  class WorkflowBuilder {
  public:
      /** @brief Virtual destructor for polymorphic builder use. */
      virtual ~WorkflowBuilder() {}

      /** @brief Add a caller-owned work item to the current stage or sub-process. */
      virtual void addWorkItem(WorkItem* item) = 0;

      /** @brief Create a definition-owned work item and add it to the current stage or sub-process. */
      virtual WorkItem* createWorkItem(const std::string& name,
                                       const std::string& requiredRole = std::string()) = 0;

      /** @brief Add a caller-owned role to the workflow being built. */
      virtual void addRole(Role<void>* role) = 0;

      /** @brief Create (or find) a definition-owned role with this name. */
      virtual Role<void>* createRole(const std::string& name) = 0;

      /** @brief Register a caller-owned participant with the workflow being built. */
      virtual void addParticipant(Participant* participant) = 0;

      /** @brief Create a definition-owned participant, optionally with a first role. */
      virtual Participant* createParticipant(const std::string& name,
                                             const std::string& role = std::string()) = 0;

      /** @brief Make @p after depend on @p before. */
      virtual void addDependency(WorkItem* before, WorkItem* after) = 0;

      /** @brief Select the assignment algorithm for the definition being built. */
      virtual void setAssignmentStrategy(AssignmentStrategy* strategy) = 0;

      /** @brief Configure escalation handling for the definition being built. */
      virtual void setEscalationHandler(Handler* handler) = 0;

      /** @brief Decorate a work item with priority during workflow construction. */
      virtual void addPriority(WorkItem* item, int priority) = 0;

      /** @brief Start a nested sub-process; later items are added inside it. */
      virtual void beginSubProcess() = 0;

      /** @brief Close the innermost sub-process opened by beginSubProcess(). */
      virtual void endSubProcess() = 0;

      /** @brief Return the finished definition and transfer ownership to the caller. */
      virtual WorkflowDefinition* getResult() = 0;
  };
}
#endif // TIVIDY_WORKFLOW_BUILDER_H
