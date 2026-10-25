#ifndef INPROGRESSSTATE_H
#define INPROGRESSSTATE_H

class InProgressState : WorkflowState {


public:
	void escalate(WorkflowInstance* instance);

	void resolve(WorkflowInstance* instance);

	void close(WorkflowInstance* instance);
};

#endif
