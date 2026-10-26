#ifndef CONCRETEWORKFLOWBUILDER_H
#define CONCRETEWORKFLOWBUILDER_H

#include "WorkflowBuilder.h"
#include "Role.h" // Include Role.h so the compiler knows what Role<T> is


namespace tividy {
class WorkflowDefinition;

class ConcreteWorkflowBuilder : public WorkflowBuilder {
	public:
		WorkflowDefinition* definition;

		void addWorkItem(WorkItem* item);

		// 1. Make this a proper member template function
		template <typename T>
		void addRole(Role<T>* role) {
			// Because it's a template, define it right here in the header!
			//CLASSES THAT UTILIZE TEMPLATE CLASS OBJECTS MUST DEFINE A FUNCTION CONTAINING THE 
			// TEMPLATE CLASSES OBJECT IN THE HEADER FILE FOR THE C++11 COMPILER TO CORRECTLY LINK THE .H & .CPP
			//DO NOT REMOVE PLEASE 
			// TODO - implement ConcreteWorkflowBuilder::addRole
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
