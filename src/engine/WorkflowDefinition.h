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
class WorkflowDefinition {
public:
    WorkflowDefinition();
    void setRoot(WorkflowComponent* component);
    WorkflowComponent* getRoot() const;
    void setAssignmentStrategy(AssignmentStrategy* strategy);
    AssignmentStrategy* getAssignmentStrategy() const;
    Participant* selectAssignee(WorkItem* item, std::vector<Participant*> participants) const;
    void setEscalationHandler(Handler* handler);
    WorkflowInstance* createInstance();
    void handleEscalation(WorkflowInstance* instance);
private:
    std::string name;
    WorkflowComponent* root;
    AssignmentStrategy* strategy;
    Handler* escalationHandler;
};
}
#endif
