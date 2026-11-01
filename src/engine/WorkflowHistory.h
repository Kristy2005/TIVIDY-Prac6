#ifndef WORKFLOWHISTORY_H
#define WORKFLOWHISTORY_H

#include "WorkflowMemento.h"

#include <vector>
#include <string>


namespace tividy {
/**
 * @brief Stores pointers to saved workflow mementos in insertion order.
 *
 * This class does not delete the mementos it stores; their lifetime remains the
 * caller's responsibility. getLast() returns the stored pointer, not a copy.
 *
 */


/**
 * @brief One readable audit event belonging to a workflow instance.
 */
struct AuditEvent {
    std::string time;
    std::string action;
    std::string workItem;
    std::string participant;

    AuditEvent(const std::string& eventTime,const std::string& eventAction,const std::string& eventWorkItem = "",const std::string& eventParticipant = "")
        : time(eventTime),action(eventAction),workItem(eventWorkItem),participant(eventParticipant) {}
};


class WorkflowHistory {
private:
    /** @brief Saved memento pointers in insertion order; */
	std::vector<WorkflowMemento*> mementos;

     /**
      * @brief Readable audit events recorded for one workflow instance.
      */
     std::vector<AuditEvent> events;

public:
	/**
     * @brief Append a memento pointer to the history.
     * @param memento Memento to store; must not be null and must remain valid.
     * @throws std::invalid_argument If @p memento is null.
     */
	void save(WorkflowMemento* memento);

	/**
     * @brief Retrieve the most recently saved memento.
     * @return The stored pointer; ownership is not transferred.
     * @throws std::logic_error If the history is empty.
     */
	WorkflowMemento* getLast();

     /**
      * @brief Records a readable event in this workflow history.
      * @param action Description of what happened.
      * @param workItem Related work item name, or empty if not applicable.
      * @param participant Related participant name, or empty if not applicable.
      */
     void recordEvent(
     const std::string& action,
     const std::string& workItem = "",
     const std::string& participant = ""
     );

     /**
      * @brief Returns all audit events in the order they occurred.
      * @return Read-only list of recorded audit events.
      */
     const std::vector<AuditEvent>& getEvents() const;

     /**
      * @brief Produces readable multi-line audit history.
      * @return Formatted audit history text.
      */
     std::string describeEvents() const;
};

}

#endif
