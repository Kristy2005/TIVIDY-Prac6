#ifndef TIVIDY_WORKFLOW_DEFINITION_H
#define TIVIDY_WORKFLOW_DEFINITION_H
#include <string>
#include <vector>
#include "WorkflowComponent.h"
#include "AssignmentStrategy.h"
#include "Handler.h"
namespace tividy {
class Participant;
class WorkItem;
class WorkflowInstance;

/**
 * @brief Blueprint used to configure and create workflow instances.
 *
 * A definition holds the work structure (root component), the assignment
 * strategy, the escalation handler, the roles and the participants that take
 * part in workflows built from it.
 *
 * @par Ownership
 * The root component, assignment strategy and escalation handler are
 * non-owning unless set with setOwnedRoot(). Roles and participants are
 * non-owning unless added with takeOwnership = true. Components handed to
 * adoptComponent() are always owned. Everything the definition owns is
 * deleted in its destructor. Every instance created from a definition must
 * be deleted before the definition, because instances point at the
 * definition's participants.
 *
 * @par Participants
 * Participants registered here are copied (as pointers) into every instance
 * created afterwards by createInstance(). The same Participant object is
 * shared by those instances, so its workload counts work across all of them.
 * Participants added to a definition after an instance exists are not added
 * to that instance; use WorkflowInstance::addParticipant for that.
 *
 * createInstance() returns a newly allocated instance; the caller is
 * responsible for deleting it.
 *
 * Copying is disabled because a copy would own the same objects twice.
 */

class WorkflowDefinition {
public:
    /** @brief Construct a named definition with no root, assignment strategy or handler. */
    WorkflowDefinition(const std::string& name);

    /** @brief Default constructor (empty name). */
    WorkflowDefinition();

    /** @brief Return the definition's name (empty if none was given). */
    const std::string& getName() const;

    /** @brief Change the definition's name. */
    void setName(const std::string& newName);

    /**
     * @brief Set the root component that represents this workflow.
     * @param component Root component to retain without taking ownership; may be null.
     */
    void setRoot(WorkflowComponent* component);

    /**
     * @brief Get the configured root component.
     * @return The non-owning root pointer, or null when none is configured.
     */
    WorkflowComponent* getRoot() const;

    /**
     * @brief Configure the assignment strategy.
     * @param strategy Strategy to retain without taking ownership; may be null.
     */
    void setAssignmentStrategy(AssignmentStrategy* strategy);

    /**
     * @brief Get the configured assignment strategy.
     * @return The non-owning strategy pointer, or null when none is configured.
     */
    AssignmentStrategy* getAssignmentStrategy() const;

    /**
     * @brief Ask the configured strategy to select an assignee for an item.
     * @param item Work item to route; strategy-specific validation applies.
     * @param participants Candidate participants, considered by the strategy.
     * @return Selected participant, or null if no strategy or suitable candidate exists.
     */
    Participant* selectAssignee(WorkItem* item, std::vector<Participant*> participants) const;

    /**
     * @brief Ask the configured strategy to select an assignee in an instance.
     * @param instance Workflow instance used for instance-specific progress checks.
     * @param item Work item to route; it must belong to the supplied instance.
     * @param participants Candidate participants considered by the strategy.
     * @return Selected participant, or nullptr if no strategy/candidate is suitable.
     *
     * The strategy's instance-aware overload is called. Its base implementation
     * returns nullptr so existing strategies that only implement the original
     * overload remain source-compatible, but must override this overload to
     * participate in instance-aware assignment.
     */
    Participant* selectAssignee(
        WorkflowInstance* instance,
        WorkItem* item,
        std::vector<Participant*> participants
    ) const;

    /**
     * @brief Configure the handler that receives escalation requests.
     * @param handler First handler in the chain; retained as a non-owning pointer.
     */
    void setEscalationHandler(Handler* handler);

    /**
     * @brief Allocate a workflow instance based on this definition.
     *
     * The new instance clones the work tree and registers this definition's
     * participants.
     *
     * @return Newly allocated instance, initially in the Submitted state; caller owns it.
     */
    WorkflowInstance* createInstance();

    /**
     * @brief Forward an escalation request to the configured handler chain.
     * @param instance Workflow instance to pass to the chain; must not be null.
     * @throws std::invalid_argument If @p instance is null.
     * @throws std::logic_error If no escalation handler is configured.
     */
    void handleEscalation(WorkflowInstance* instance);

    /**
     * @brief Creates a cloned work tree for a new independent workflow instance.
     */
    WorkflowComponent* createWorkTree() const;

    /**
     * @brief Sets a root component that is owned by this workflow definition.
     * @param component Root workflow component whose ownership is transferred to this definition.
     */
    void setOwnedRoot(WorkflowComponent* component);

    /**
     * @brief Transfer ownership of a component to this definition.
     *
     * The component is deleted with the definition. It is not added to the
     * work tree; the caller (normally the builder) places it in a stage.
     * If the call throws, the component has already been deleted.
     *
     * @param component Component to own; must not be null.
     * @throws std::invalid_argument If @p component is null.
     */
    void adoptComponent(WorkflowComponent* component);

    /**
     * @brief Add a role to the workflow definition.
     *
     * Adding the same pointer again does nothing.
     *
     * @param role Role pointer; must not be null.
     * @param takeOwnership When true the definition deletes the role.
     * @throws std::invalid_argument If @p role is null, or a different role
     *         with the same non-empty name is already present.
     */
    void addRole(Role<void>* role, bool takeOwnership = false);

    /**
     * @brief Register a participant for workflows built from this definition.
     *
     * Adding the same pointer again does nothing.
     *
     * @param participant Participant to register; must not be null and must
     *        have a non-blank name unique within this definition.
     * @param takeOwnership When true the definition deletes the participant.
     * @throws std::invalid_argument If @p participant is null, has a blank
     *         name, or another participant already has that name.
     */
    void addParticipant(Participant* participant, bool takeOwnership = false);

    /**
     * @brief Return the participants registered with this definition.
     * @return Copy of the pointer list, in the order they were added; non-owning.
     */
    std::vector<Participant*> getParticipants() const;

    /**
     * @brief Find a registered participant by name.
     * @param participantName Exact name to look for.
     * @return The participant, or nullptr when none has that name.
     */
    Participant* findParticipant(const std::string& participantName) const;

    /**
     * @brief Destructor. Deletes the owned root, components, roles and participants.
     */
    ~WorkflowDefinition();

    /**
     * @brief Return the roles associated with this workflow definition.
     * @return Vector of role pointers; non-owning and may be empty.
     */
    std::vector<Role<void>*> getRoles() const;

private:
    WorkflowDefinition(const WorkflowDefinition&);
    WorkflowDefinition& operator=(const WorkflowDefinition&);

    /** @brief Definition name */
    std::string name;

    /** @brief Root workflow component. */
    WorkflowComponent* root;

    /** @brief Indicates whether this definition owns and must delete its root component. */
    bool ownsRoot;

    /** @brief Configured assignment strategy; non-owning. */
    AssignmentStrategy* strategy;

    /** @brief First escalation handler; non-owning. */
    Handler* escalationHandler;

    /** @brief Configured roles. */
    std::vector<Role<void>*> roles;

    /** @brief Registered participants. */
    std::vector<Participant*> participants;

    /** @brief Components deleted with this definition (items, sub-processes, decorators). */
    std::vector<WorkflowComponent*> ownedComponents;

    /** @brief Roles deleted with this definition. */
    std::vector<Role<void>*> ownedRoles;

    /** @brief Participants deleted with this definition. */
    std::vector<Participant*> ownedParticipants;
};
}
#endif