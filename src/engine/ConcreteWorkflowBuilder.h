#ifndef CONCRETEWORKFLOWBUILDER_H
#define CONCRETEWORKFLOWBUILDER_H

#include "WorkflowBuilder.h"
#include "Role.h" // Include Role.h so the compiler knows what Role<T> is


namespace tividy {
  class WorkflowDefinition;

  /**
   * @brief Concrete builder that configures a WorkflowDefinition.
   *
   * This builder owns its in-progress definition until getResult() transfers that
   * pointer to the caller. Assignment strategies are non-owning configuration
   * dependencies and must outlive the resulting definition.
   */

  class ConcreteWorkflowBuilder : public WorkflowBuilder {
  public:
    /** @brief Construct a builder with a new empty definition. */
      ConcreteWorkflowBuilder();

      /** @brief Deep-copy the in-progress definition; strategy dependencies remain non-owning. */
      ConcreteWorkflowBuilder(const ConcreteWorkflowBuilder& other);

      /** @brief Copy assignment with safe replacement of the owned definition. */
      ConcreteWorkflowBuilder& operator=(const ConcreteWorkflowBuilder& other);

      /** @brief Destroy any definition that has not been transferred by getResult(). */
      virtual ~ConcreteWorkflowBuilder();

    /** @brief Preservation of current project interface */
    WorkflowDefinition* definition;

    void addWorkItem(WorkItem* item);

    void addRole(Role<void>* role) override;

    // 1. Make this a proper member template function
    template <typename T>
    void addRole(Role<T>* role) {
      // Because it's a template, define it right here in the header!
      //CLASSES THAT UTILIZE TEMPLATE CLASS OBJECTS MUST DEFINE A FUNCTION CONTAINING THE 
      // TEMPLATE CLASSES OBJECT IN THE HEADER FILE FOR THE C++11 COMPILER TO CORRECTLY LINK THE .H & .CPP
      //DO NOT REMOVE PLEASE 
      // TODO - implement ConcreteWorkflowBuilder::addRole
      (void)role;
      throw "Not yet implemented";
    }

    void addDependency(WorkItem* before, WorkItem* after);
    void setAssignmentStrategy(AssignmentStrategy* strategy);
    void setEscalationHandler(Handler* handler);
    void addPriority(WorkItem* item, int priority);
    WorkflowDefinition* getResult();
  };
}
#endif // CONCRETEWORKFLOWBUILDER_H
