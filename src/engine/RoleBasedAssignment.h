#ifndef TIVIDY_ROLE_BASED_ASSIGNMENT_H
#define TIVIDY_ROLE_BASED_ASSIGNMENT_H

#include "AssignmentStrategy.h"

namespace tividy {

class WorkflowInstance;

/**

* @brief Assignment strategy that selects the first participant with the required role.
*
* The required role is read from WorkItem. Candidates are considered in the
* order supplied by the caller, null candidates are ignored, and the strategy
* returns nullptr if the item has no required role or no candidate matches.
  */
  class RoleBasedAssignment : public AssignmentStrategy {
  public:
  virtual Participant* assign(
  WorkItem* item,
  std::vector<Participant*> participants
  );

  virtual Participant* assign(
  WorkflowInstance* instance,
  WorkItem* item,
  std::vector<Participant*> participants
  );
  };

}

#endif // TIVIDY_ROLE_BASED_ASSIGNMENT_H
