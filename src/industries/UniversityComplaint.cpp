#include <iostream>
#include <stdexcept>
#include <vector>

#include "engine/WorkflowDefinition.h"
#include "engine/WorkflowInstance.h"
#include "engine/WorkItem.h"
#include "engine/Participant.h"
#include "engine/RoleBasedAssignment.h"
#include "engine/ApprovalHandler.h"
#include "engine/EscalationHandler.h"
#include "engine/FinalDecisionHandler.h"
#include "engine/TIVIDYFacade.h"
#include "engine/Console.h"
#include "engine/ConcreteWorkflowBuilder.h"
#include "InteractiveWorkflowSession.h"

void runUniversityComplaintScenario()
{
    using namespace tividy;
    using namespace tividy::console;

    std::cout << MAGENTA << BOLD << "\n========================================\n";
    std::cout << " UNIVERSITY: STUDENT COMPLAINT WORKFLOW\n";
    std::cout << "========================================\n" << RESET;

    WorkItem recordComplaint("Record student complaint", "Faculty Administrator");
    WorkItem validateComplaint("Validate complaint and supporting evidence", "Faculty Administrator");
    WorkItem investigateComplaint("Investigate disputed mark", "Department Representative");
    WorkItem reviewOutcome("Review investigation outcome", "Faculty Manager");
    WorkItem notifyStudent("Notify student of outcome", "Student Support Officer");
    WorkItem closeCase("Close complaint case", "Faculty Administrator");

    Participant administrator("Lerato - Faculty Office");
    administrator.addRole("Faculty Administrator");

    Participant departmentRep("Mr Dlamini - Department");
    departmentRep.addRole("Department Representative");

    Participant facultyManager("Ms Naidoo - Faculty Manager");
    facultyManager.addRole("Faculty Manager");

    Participant studentSupport("Kabelo - Student Support");
    studentSupport.addRole("Student Support Officer");

    std::vector<Participant*> staff;
    staff.push_back(&administrator);
    staff.push_back(&departmentRep);
    staff.push_back(&facultyManager);
    staff.push_back(&studentSupport);

    RoleBasedAssignment roleAssignment;
    ApprovalHandler initialReview;
    EscalationHandler managerEscalation;
    FinalDecisionHandler finalReview;
    initialReview.setSuccessor(&managerEscalation);
    managerEscalation.setSuccessor(&finalReview);

    ConcreteWorkflowBuilder builder;
    builder.addDependency(&recordComplaint, &validateComplaint);
    builder.addDependency(&validateComplaint, &investigateComplaint);
    builder.addDependency(&investigateComplaint, &reviewOutcome);
    builder.addDependency(&reviewOutcome, &notifyStudent);
    builder.addDependency(&notifyStudent, &closeCase);
    builder.setAssignmentStrategy(&roleAssignment);
    builder.setEscalationHandler(&initialReview);

    WorkflowDefinition* definition = builder.getResult();
    if (definition == nullptr) {
        throw std::runtime_error("University complaint workflow definition could not be created");
    }

    TIVIDYFacade facade;
    WorkflowInstance* instance = nullptr;
    try {
        instance = facade.startWorkflow(definition);
        instance->startProcessing();

        char escalationChoice = 'n';
        std::cout << "\nCase: Student U-2081 disputes a recorded mark.\n";
        std::cout << "Does this complaint require senior review? (y/n): ";
        if (!(std::cin >> escalationChoice)) {
            throw std::runtime_error("Could not read the senior-review decision");
        }

        const bool requiresEscalation =
            escalationChoice == 'y' || escalationChoice == 'Y';
        if (requiresEscalation) {
            std::cout << RED << "Senior review required. Sending the case through "
                      << "ApprovalHandler, EscalationHandler and FinalDecisionHandler.\n"
                      << RESET;
            facade.escalateWork(instance);
        } else {
            std::cout << GREEN << "Senior review not required. Continue with the normal path.\n"
                      << RESET;
        }

        runInteractiveWorkflowSession(
            facade,
            definition,
            instance,
            staff,
            "UNIVERSITY - STUDENT COMPLAINT U-2081"
        );
        instance = nullptr; // The interactive session owns and deletes it.
    } catch (...) {
        if (instance != nullptr) delete instance;
        delete definition;
        throw;
    }

    delete definition;
}
