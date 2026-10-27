#ifndef TIVIDY_COMMAND_H
#define TIVIDY_COMMAND_H
#include "WorkflowMemento.h"
namespace tividy {
class WorkflowInstance;
class Command {
protected:
    WorkflowInstance* receiver;
    WorkflowMemento* previous;
    void saveState();
public:
    Command();
    explicit Command(WorkflowInstance* instance);
    virtual ~Command();
    void setReceiver(WorkflowInstance* instance);
    virtual void execute() = 0;
    virtual void undo() = 0;
};
}
#endif
