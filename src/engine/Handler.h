#ifndef HANDLER_H
#define HANDLER_H

namespace tividy {

class WorkflowInstance;

class Handler {
protected:
    Handler* successor;

public:
    Handler();
    virtual ~Handler();

    virtual void setSuccessor(Handler* handler);
    virtual void handleRequest(WorkflowInstance* instance);
};

}

#endif