#ifndef WORKFLOWHISTORY_H
#define WORKFLOWHISTORY_H

#include "WorkflowMemento.h"

#include <vector>


namespace tividy {
/**
 * @brief Stores pointers to saved workflow mementos in insertion order.
 *
 * This class does not delete the mementos it stores; their lifetime remains the
 * caller's responsibility. getLast() returns the stored pointer, not a copy.
 *
 */
class WorkflowHistory {
private:
    /** @brief Saved memento pointers in insertion order; */
	std::vector<WorkflowMemento*> mementos;

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
};

}

#endif
