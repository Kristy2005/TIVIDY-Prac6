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

    ~WorkflowMemento();

    // WorkflowMemento(const WorkflowMemento&) = delete;
    
    // WorkflowMemento& operator=(const WorkflowMemento&) = delete;
    };

}

#endif // WORKFLOWMEMENTO_H

