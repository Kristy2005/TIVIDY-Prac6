#ifndef CONCRETEWORKFLOWBUILDER_H
#define CONCRETEWORKFLOWBUILDER_H

#include "WorkflowBuilder.h"
#include "Role.h"
#include <stdexcept> // Include Role.h so the compiler knows what Role<T> is


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

      /** @brief Destroy any definition that has not been transferred by getResult(). */
      virtual ~ConcreteWorkflowBuilder();

    /** @brief Preservation of current project interface */
    WorkflowDefinition* definition;

    /**
     * @brief Add a work item to the root workflow stage.
     * @param item Work item to add; must not be null and is not owned.
     * @throws std::invalid_argument If @p item is null.
     */
    void addWorkItem(WorkItem* item);

    /**
     * @brief Validate a role argument.
     *
     * The current implementation does not store the role in the definition.
     * @param role Role pointer; must not be null.
     * @throws std::invalid_argument If @p role is null.
     */
    void addRole(Role<void>* role) override;

    // 1. Make this a proper member template function
    /**
     * @brief Template overload that validates a role argument.
     *
     * The current implementation does not store the role in the definition.
     * @tparam T Role's template parameter.
     * @param role Role pointer; must not be null.
     * @throws std::invalid_argument If @p role is null.
     */
    template <typename T>
    void addRole(Role<T>* role) {
      // Because it's a template, define it right here in the header!
      //CLASSES THAT UTILIZE TEMPLATE CLASS OBJECTS MUST DEFINE A FUNCTION CONTAINING THE 
      // TEMPLATE CLASSES OBJECT IN THE HEADER FILE FOR THE C++11 COMPILER TO CORRECTLY LINK THE .H & .CPP
      //DO NOT REMOVE PLEASE 
      if (role == nullptr) { throw std::invalid_argument("addRole: role cannot be null"); }
    }

    /**
     * @brief Add both work items to the workflow in the requested order.
     *
     * The current model does not store dependency metadata; this method simply
     * adds @p before and then @p after as children of the root stage.
     * @param before First work item; must not be null.
     * @param after Second work item; must not be null.
     * @throws std::invalid_argument If either pointer is null.
     */
    void addDependency(WorkItem* before, WorkItem* after);
    
    /**
     * @brief Configure the definition's assignment strategy.
     * @param strategy Strategy pointer to retain without ownership; may be null.
     */
    void setAssignmentStrategy(AssignmentStrategy* strategy);
    
    /**
     * @brief Configure the definition's escalation handler.
     * @param handler First handler pointer to retain without ownership; may be null.
     */
    void setEscalationHandler(Handler* handler);
    
    /**
     * @brief Validate a work item's priority argument.
     *
     * The current implementation validates the item and non-negative value but
     * does not attach a PriorityDecorator or store the priority.
     * @param item Work item to validate; must not be null.
     * @param priority Priority value; must be non-negative.
     * @throws std::invalid_argument If @p item is null or @p priority is negative.
     */
    void addPriority(WorkItem* item, int priority);
    
    /**
     * @brief Return the definition and transfer its ownership to the caller.
     * @return The owned definition pointer, or null if already transferred.
     */
    WorkflowDefinition* getResult();
  };
}
#endif // CONCRETEWORKFLOWBUILDER_H
