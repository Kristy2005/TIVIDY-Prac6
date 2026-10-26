#ifndef WORKFLOWINSTANCE_H
#define WORKFLOWINSTANCE_H

// #include "WorkflowDefinition.h"
// #include "WorkflowObserver.h"
#include "WorkflowMemento.h"
#include "WorkflowState.h"
#include <vector>

class WorkflowObserver;
class WorkflowDefinition;

/**
 * @brief Design Pattern: Builder, State, Observer, Memento, Command
 *        Participant: Product , Context, Subject, Originator, Receiver
 * A WorkflowInstance is created by the WorkflowDefinition and is held by the WorkflowDefinition as a specific instance of the workflow. 
 * The WorkflowInstance is the context for the state pattern, and it is also the subject for the observer pattern.
 * 
 * @par Each WorkflowInstance has a current state depending on where the workflow is in its lifecycle 
 * 
 * @par Each WorkflowInstance has a list of observers that are notified when the state changes
 * 
 * @par Each WorkflowInstance has a definition that defines the workflow and its components
 * 
 * @par The WorkflowInstance is the context for the state pattern, and it is also the subject for the observer pattern.
 * 
 * @see WorkflowState
 */

namespace tividy {

	class WorkflowInstance {

	private:
	/*
	* @brief The current state of the workflow instance.
	*/
		WorkflowState* currentState;
	/*
	* @brief The definition of the workflow instance.
	*/
		WorkflowDefinition* definition;
	/*
	* @brief The list of observers that are notified when the state changes.
	*/
		std::vector<WorkflowObserver*> observers;

	public:
	/*
	*@brief setter function for the current state of the workflow instance.
	*@param state The new state of the workflow instance.
	@throws std::invalid_argument if the state is null.
	*/
		void setState(WorkflowState* state);
	/*
	*@brief function to change the state of a workflow instance to the submitted state.
	*@throws std::logic_error if the current state does not allow submission.
	*/		
		void submit();
	/*
	*@brief function to change the state of a workflow instance to the active state.
	*@throws std::logic_error if the current state does not allow activation.
	*/
		void activate();
	/*
	*@brief function to change the state of a workflow instance to the in progress state.
	*@throws std::logic_error if the current state does not allow processing to start.
	*/
		void startProcessing();
	/*
	*@brief recieves the ConcreteCommand escalate and executes uses the WorkflowDefinition to escalate the current instance to the appropriate authority figure
	*/
		void escalate();
	/*
	*@brief creates a snapshot of the current WorkflowInstance state and returns a pointer to the WorkflowMemento object.
	*@return a pointer to the WorkflowMemento object.
	*/
		WorkflowMemento* createMemento();
	/*
	*@brief restores the state of the WorkflowInstance to the state stored in the WorkflowMemento object.
	*@param memento The WorkflowMemento object that contains the state to restore.
	*@throws std::invalid_argument if the memento is null.
	*/

		void restore(WorkflowMemento* memento);
	/*
	*@brief function to change the state of a workflow instance to the resolved state.
	*@throws std::logic_error if the current state does not allow resolving.
	*/
		void resolve();
	/*
	*@brief function to change the state of a workflow instance to the closed state.
	*@throws std::logic_error if the current state does not allow closing.
	*/

		void close();
	/*
	*@brief notifies all observers registered with the WorkflowInstance that the state has changed.
	*@throws std::logic_error if there are no registered observers.
	*/

		void notifyObservers();
	/*
	*@brief adds an observer to the list of observers registered with the WorkflowInstance.
	*@param observer The observer to add.
	*@throws std::invalid_argument if the observer is null.
	*/
	
		void attach(WorkflowObserver* observer);
	/*
	*@brief removes an observer from the list of observers registered with the WorkflowInstance.
	*@param observer The observer to remove.
	*@throws std::invalid_argument if the observer is null or not found in the list of observers.	
	*/

		void detach(WorkflowObserver* observer);
	};

}

#endif // WORKFLOWINSTANCE_H
