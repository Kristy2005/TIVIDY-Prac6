#ifndef WORKFLOWMEMENTO_H
#define WORKFLOWMEMENTO_H

namespace tividy {

class WorkflowState;

class WorkflowMemento {
private:

    WorkflowState* state;

public:
    WorkflowMemento();

    void setState(WorkflowState* state);

    WorkflowState* getState() const;

};

}

#endif // WORKFLOWMEMENTO_H

