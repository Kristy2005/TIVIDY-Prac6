#ifndef WORKFLOWINSTANCE_H
#define WORKFLOWINSTANCE_H

#include "WorkflowDefinition.h"
#include "WorkflowMemento.h"
#include "WorkflowHistory.h"

#include <map>
#include <string>
#include <vector>

namespace tividy {

class Participant;
class WorkflowObserver;
class WorkflowState;
class WorkItem;

/**
 * @brief Progress of one work item within a particular workflow instance.
 *
 * A cancelled item remains in the internal progress list so that its history
 * and dependencies can be validated, but it is excluded from the active work
 * shown by the Facade and from progress percentages.
 */
struct WorkItemProgress {
    WorkItem* item;
    bool completed;
    bool cancelled;

    explicit WorkItemProgress(WorkItem* workItem)
        : item(workItem), completed(false), cancelled(false) {}
};

/**
 * @brief One running execution of a WorkflowDefinition.
 *
 * Plays three pattern roles:
 * - State pattern: Context. Holds the current WorkflowState and changes it
 *   as the case moves through its lifecycle.
 * - Observer pattern: Subject. Keeps a list of observers and notifies them
 *   when the state changes.
 * - Memento pattern: Originator. Creates mementos of its state and restores
 *   from them.
 *
 * @par Runtime changes
 * Work added, cancelled or altered through this class belongs to this
 * instance only. The source WorkflowDefinition is never changed. Starting a
 * second instance therefore produces the original definition's work structure.
 *
 * @par Ownership
 * - The instance does not own its definition. The definition must outlive
 *   every instance created from it.
 * - The instance owns its cloned workflow tree, dynamically added work items,
 *   and participants explicitly added with takeOwnership=true.
 * - Participants supplied with takeOwnership=false and observers are
 *   non-owning pointers; their owners must keep them alive while registered.
 * - State pointers are non-owning; the instance never deletes its current or
 *   replaced state objects.
 *
 * @warning Not safe to copy, because it holds raw pointers. Do not pass it
 *          by value or assign it.
 *
 * @see WorkflowDefinition, WorkflowState, WorkflowObserver, WorkflowMemento
 */
class WorkflowInstance {
private:
    /** @brief Audit and memento history belonging only to this workflow instance. */
    WorkflowHistory history;

    /** @brief Maps definition work items to this instance's cloned work items. */
    std::map<const WorkItem*, WorkItem*> prototypeToInstance;

    /** @brief The state the instance is in now. Never null after construction. */
    WorkflowState* currentState;

    /** @brief The blueprint this instance was created from. Not owned. */
    WorkflowDefinition* definition;

    /** @brief Observers to notify on a state change. Not owned. */
    std::vector<WorkflowObserver*> observers;

    /** @brief Progress for cloned and dynamically added work items. */
    std::vector<WorkItemProgress> workItemProgress;

    /** @brief Independent cloned work structure used by this workflow instance. */
    WorkflowComponent* workTree;

    /** @brief Participants available to this instance. */
    std::vector<Participant*> participants;

    /** @brief Dynamically added work items owned by this instance. */
    std::vector<WorkItem*> ownedWorkItems;

    /** @brief Participants whose ownership was transferred to this instance. */
    std::vector<Participant*> ownedParticipants;

    /** @brief Current assignee for each instance-owned work item. */
    std::map<WorkItem*, Participant*> assignedParticipants;

    /** @brief Resolve a definition pointer to its clone and validate membership. */
    WorkItem* resolveWorkItem(WorkItem* item) const;

    /** @brief Validate that a live instance can still be changed. */
    void ensureEditable() const;

    /** @brief Release and forget any assignment currently held for an item. */
    void releaseAssignment(WorkItem* item);

public:
    /** @brief Returns this instance's audit and memento history. */
    const WorkflowHistory& getHistory() const;

    explicit WorkflowInstance(WorkflowDefinition* definition = nullptr);
    WorkflowInstance(const WorkflowInstance&) = delete;
    WorkflowInstance& operator=(const WorkflowInstance&) = delete;

    /** @brief Destroys the workflow instance and all objects it owns. */
    ~WorkflowInstance();

    /** @brief Mark a work item as complete for this instance only. */
    void completeWorkItem(WorkItem* item);

    /** @brief Check whether a work item is complete in this instance. */
    bool isWorkItemComplete(WorkItem* item) const;

    /** @brief Check whether a work item was cancelled in this instance. */
    bool isWorkItemCancelled(WorkItem* item) const;

    /** @brief Return all progress records, including cancelled records for audit purposes. */
    const std::vector<WorkItemProgress>& getWorkItemProgress() const;

    /** @brief Resolve a definition item pointer to this instance's cloned item. */
    WorkItem* getInstanceWorkItem(WorkItem* item) const;

    /**
     * @brief Add an instance-owned work item to a running workflow.
     * @param name Non-empty, unique name for the new item.
     * @param requiredRole Role required to perform the new item.
     * @param dependency Optional existing item that must be completed first.
     * @return Pointer to the new work item, owned by this instance.
     * @throws std::invalid_argument For invalid names, roles or dependencies.
     * @throws std::logic_error If the instance is no longer editable.
     */
    WorkItem* addWorkItem(
        const std::string& name,
        const std::string& requiredRole,
        WorkItem* dependency = nullptr
    );

    /**
     * @brief Cancel an incomplete work item in this instance only.
     * @throws std::invalid_argument If the item is not part of this instance.
     * @throws std::logic_error If it is completed/cancelled or has outstanding dependents.
     */
    void cancelWorkItem(WorkItem* item);

    /** @brief Change a live work item's required role for this instance only. */
    void changeWorkItemRole(WorkItem* item, const std::string& requiredRole);

    /** @brief Change a live work item's non-negative numeric priority. */
    void changeWorkItemPriority(WorkItem* item, int priority);

    /**
     * @brief Register a participant for this instance.
     * @param participant Participant to add.
     * @param takeOwnership When true, the instance deletes the participant.
     */
    void addParticipant(Participant* participant, bool takeOwnership = false);

    /** @brief Return participant pointers available to this instance. */
    const std::vector<Participant*>& getParticipantPointers() const;

    /** @brief Assign or reassign an item to a participant in this instance. */
    void assignParticipant(WorkItem* item, Participant* participant);

    /** @brief Return the current assignee of an item, or null if unassigned. */
    Participant* getAssignee(WorkItem* item) const;

    /** @brief Return the current state. The returned state pointer is non-owning. */
    WorkflowState* getState() const;

    /** @brief Return the non-owning definition associated with this instance. */
    WorkflowDefinition* getDefinition() const;

    /** @brief Check whether all dependencies for a work item are complete. */
    bool areDependenciesComplete(WorkItem* item) const;

    /** @brief True when at least one non-cancelled item exists and all such items are complete. */
    bool areAllWorkItemsComplete() const;

    /** @brief Replaces the current state and notifies observers. */
    void setState(WorkflowState* state);

    /** @brief Submit the case. New instances begin in SubmittedState. */
    void submit();

    /** @brief Move the instance from Submitted to Active. */
    void activate();

    /** @brief Start work on the case, moving it to In Progress. */
    void startProcessing();

    /** @brief Escalate an in-progress case through its definition's handler chain. */
    void escalate();

    /** @brief Create a state memento; the caller owns the returned pointer. */
    WorkflowMemento* createMemento();

    /** @brief Restore the state stored in a memento. */
    void restore(WorkflowMemento* memento);

    /** @brief Mark the case as successfully resolved. */
    void resolve();

    /** @brief Close the case without resolving it. */
    void close();

    /** @brief Notify every registered observer of a state change. */
    void notifyObservers();

    /** @brief Register a non-owning observer. */
    void attach(WorkflowObserver* observer);

    /** @brief Remove a non-owning observer. */
    void detach(WorkflowObserver* observer);

    /** @brief Return participant names followed by their role names (legacy format). */
    const std::vector<std::string> getParticipants() const;
};

} // namespace tividy

#endif // WORKFLOWINSTANCE_H
