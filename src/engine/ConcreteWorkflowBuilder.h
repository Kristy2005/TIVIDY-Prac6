#ifndef CONCRETEWORKFLOWBUILDER_H
#define CONCRETEWORKFLOWBUILDER_H

#include "WorkflowBuilder.h"
#include "Role.h"

#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace tividy {
  class WorkflowDefinition;
  class WorkflowComponent;
  class PriorityDecorator;

  /**
   * @brief Concrete builder that configures a WorkflowDefinition.
   *
   * Builder pattern: ConcreteBuilder.
   *
   * @par Two modes
   * - Creating mode (default, name or no-argument constructors): the builder
   *   owns a new definition until getResult() hands it to the caller. The
   *   builder is reusable: after getResult() the next step starts a fresh
   *   definition.
   * - Configuring mode (constructor taking a WorkflowDefinition*): the builder
   *   edits an existing definition it does not own. getResult() returns that
   *   same pointer and does not release it. TIVIDYFacade uses this mode.
   *
   * @par Structure
   * Items go into the root WorkflowStage, which the builder creates on demand.
   * beginSubProcess()/endSubProcess() open and close nested SubProcess nodes
   * (any depth); items added in between go into the innermost one.
   * getResult() throws if a sub-process is still open.
   *
   * @par Ownership
   * Objects passed to add...() stay with the caller. Objects made by
   * create...() and the stages, sub-processes and priority decorators the
   * builder makes are owned by the definition. Assignment strategies and
   * handlers are non-owning and must outlive the definition.
   */

  class ConcreteWorkflowBuilder : public WorkflowBuilder {
  public:
    /** @brief Construct a builder with a new empty, unnamed definition. */
      ConcreteWorkflowBuilder();

    /** @brief Construct a builder with a new empty definition called @p name. */
      ConcreteWorkflowBuilder(const std::string& name);

    /**
     * @brief Construct a builder that edits an existing definition.
     * @param existing Definition to configure; not owned, must outlive the builder's use.
     * @throws std::invalid_argument If @p existing is null.
     */
      explicit ConcreteWorkflowBuilder(WorkflowDefinition* existing);

      /** @brief Destroy any owned definition that has not been transferred by getResult(). */
      virtual ~ConcreteWorkflowBuilder();

    /** @brief The definition being built (public to preserve the current project interface). */
    WorkflowDefinition* definition;

    /**
     * @brief Add a caller-owned work item to the current stage or sub-process.
     *
     * Adding the same item to the same place twice does nothing.
     * @param item Work item to add; must not be null and is not owned.
     * @throws std::invalid_argument If @p item is null or already belongs to a
     *         different stage or sub-process.
     */
    void addWorkItem(WorkItem* item) override;

    /**
     * @brief Create a work item owned by the definition and add it.
     * @param name Non-blank task name.
     * @param requiredRole Role required to perform it; may be empty.
     * @return The new item (owned by the definition).
     * @throws std::invalid_argument If @p name is blank.
     */
    WorkItem* createWorkItem(const std::string& name,
                             const std::string& requiredRole = std::string()) override;

    /**
     * @brief Add a caller-owned role to the definition.
     * @param role Role pointer; must not be null. Adding it twice does nothing.
     * @throws std::invalid_argument If @p role is null or another role has the same name.
     */
    void addRole(Role<void>* role) override;

    /**
     * @brief Create a definition-owned role, or return the existing one with that name.
     * @param name Non-blank role name.
     * @return The role (owned by the definition).
     * @throws std::invalid_argument If @p name is blank.
     */
    Role<void>* createRole(const std::string& name) override;

    /**
     * @brief Register a caller-owned participant with the definition.
     *
     * Instances created from the definition afterwards include this participant.
     * @param participant Participant; must not be null, with a unique non-blank name.
     * @throws std::invalid_argument On null, blank or duplicate name.
     */
    void addParticipant(Participant* participant) override;

    /**
     * @brief Create a definition-owned participant.
     * @param name Non-blank, unique participant name.
     * @param role Optional first role. The role is also registered on the
     *        definition (created if missing) so getRoles() stays complete.
     * @return The new participant (owned by the definition).
     * @throws std::invalid_argument If @p name is blank or already used.
     */
    Participant* createParticipant(const std::string& name,
                                   const std::string& role = std::string()) override;

    /**
     * @brief Make @p after depend on @p before.
     *
     * Both items are added to the work structure if they are not already in
     * it; items already placed are left where they are. Nothing is changed if
     * the call throws.
     * @param before Prerequisite; must not be null.
     * @param after Dependent item; must not be null.
     * @throws std::invalid_argument If either is null, they are the same item,
     *         or the dependency would create a cycle.
     */
    void addDependency(WorkItem* before, WorkItem* after) override;

    /**
     * @brief Configure the definition's assignment strategy.
     * @param strategy Strategy pointer to retain without ownership; may be null.
     */
    void setAssignmentStrategy(AssignmentStrategy* strategy) override;

    /**
     * @brief Configure the definition's escalation handler.
     * @param handler First handler pointer to retain without ownership; may be null.
     */
    void setEscalationHandler(Handler* handler) override;

    /**
     * @brief Give a work item a priority and wrap it in a PriorityDecorator.
     *
     * The value is stored on the item (so instance clones carry it) and the
     * item's place in the tree is taken by a decorator owned by the
     * definition. Calling it again for the same item only updates the value.
     * The item is added to the work structure first if it is not already in it.
     * @param item Work item; must not be null.
     * @param priority Non-negative priority (higher means more urgent).
     * @throws std::invalid_argument If @p item is null or @p priority is negative.
     */
    void addPriority(WorkItem* item, int priority) override;

    /**
     * @brief Open a nested sub-process inside the current stage or sub-process.
     */
    void beginSubProcess() override;

    /**
     * @brief Close the innermost open sub-process.
     * @throws std::logic_error If no sub-process is open.
     */
    void endSubProcess() override;

    /**
     * @brief Return the finished definition.
     *
     * Creating mode: ownership passes to the caller and the builder is reset.
     * A second call returns null until new steps start a fresh definition.
     * Configuring mode: returns the edited definition, still not owned.
     * @return The definition, or null if already transferred.
     * @throws std::logic_error If a sub-process is still open.
     */
    WorkflowDefinition* getResult() override;

  private:
    ConcreteWorkflowBuilder(const ConcreteWorkflowBuilder&);
    ConcreteWorkflowBuilder& operator=(const ConcreteWorkflowBuilder&);

    /** @brief True when the builder deletes @c definition itself. */
    bool ownsDefinition;

    /** @brief Name for definitions this builder creates. */
    std::string name;

    /** @brief Open sub-processes, innermost last. */
    std::vector<WorkflowComponent*> openSubProcesses;

    /** @brief Stage or sub-process that holds each component placed by this builder. */
    std::map<const WorkflowComponent*, WorkflowComponent*> parents;

    /** @brief Decorator that replaced each prioritised item. */
    std::map<const WorkItem*, PriorityDecorator*> decorators;

    /** @brief Create a fresh owned definition if none is held. */
    void ensureDefinition();

    /** @brief Return the root stage, creating it if needed. */
    WorkflowComponent* rootStage();

    /** @brief The stage or sub-process new components are added to. */
    WorkflowComponent* currentComposite();

    /** @brief Add a component to the current composite and record its parent. */
    void attach(WorkflowComponent* component);

    /** @brief Add an item only if this builder has not placed it yet. */
    void ensureInTree(WorkItem* item);
  };
}
#endif // CONCRETEWORKFLOWBUILDER_H
