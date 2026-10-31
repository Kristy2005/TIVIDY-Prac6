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
 * The root component, assignment strategy, and escalation handler are stored
 * as non-owning pointers. Their owners must keep them alive while this
 * definition or any instance using it needs them. createInstance() returns a
 * newly allocated instance; the caller is responsible for deleting it.
 *
 * The currently declared name field is reserved and is not used by the
 * implementation.
 *
 */

class WorkflowDefinition {
public:
    /** @brief Construct a definition with no root, assignment strategy or handler. */
    WorkflowDefinition();

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
     * @brief Destructor deleting root.
     */
    ~WorkflowDefinition();
private:
    /**
     * @brief Indicates whether this definition owns and must delete its root component.
     */
    bool ownsRoot;

    /** @brief Definition name */
    std::string name;

    /** @brief Root workflow component; non-owning. */
    WorkflowComponent* root;

    /** @brief Configured assignment strategy; non-owning. */
    AssignmentStrategy* strategy;

    /** @brief First escalation handler; non-owning. */
    Handler* escalationHandler;
};
}
#endif
