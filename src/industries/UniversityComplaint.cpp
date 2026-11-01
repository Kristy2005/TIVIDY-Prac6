#include <iostream>
#include <stdexcept>
#include <vector>

#include "engine/WorkflowDefinition.h"
#include "engine/WorkflowInstance.h"
#include "engine/WorkItem.h"
#include "engine/Participant.h"
#include "engine/RoleBasedAssignment.h"
#include "engine/ProgressObserver.h"
#include "engine/ApprovalHandler.h"
#include "engine/EscalationHandler.h"
#include "engine/FinalDecisionHandler.h"
#include "engine/TIVIDYFacade.h"
#include "engine/Console.h"
#include "engine/ConcreteWorkflowBuilder.h"
#include "InteractiveWorkflowSession.h"

namespace {

bool allWorkItemsCompleted(const tividy::WorkflowInstance* instance)
{
    if (instance == nullptr) {
        return false;
    }

    const std::vector<tividy::WorkItemProgress>& progress =
        instance->getWorkItemProgress();

    if (progress.empty()) {
        return false;
    }

    for (std::size_t i = 0; i < progress.size(); ++i) {
        if (!progress[i].completed) {
            return false;
        }
    }

    return true;
}

} // namespace

void runUniversityComplaintScenario()
{
    using namespace tividy;
    using namespace tividy::console;

    std::cout << MAGENTA << BOLD<< "\n========================================\n";
    std::cout << " UNIVERSITY: STUDENT COMPLAINT WORKFLOW\n";
    std::cout << "========================================\n"<< RESET;

    WorkItem recordComplaint( "Record student complaint","Faculty Administrator");
    WorkItem validateComplaint("Validate complaint and supporting evidence","Faculty Administrator");
    WorkItem investigateComplaint("Investigate disputed mark","Department Representative");
    WorkItem reviewOutcome("Review investigation outcome","Faculty Manager");
    WorkItem notifyStudent("Notify student of outcome","Student Support Officer");
    WorkItem closeCase("Close complaint case","Faculty Administrator");

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

    // Required dependency order.
    builder.addDependency(&recordComplaint,&validateComplaint);
    builder.addDependency(&validateComplaint,&investigateComplaint);
    builder.addDependency(&investigateComplaint,&reviewOutcome);
    builder.addDependency(&reviewOutcome,&notifyStudent);
    builder.addDependency(&notifyStudent,&closeCase
    );

    builder.setAssignmentStrategy(&roleAssignment);
    builder.setEscalationHandler(&initialReview);

    WorkflowDefinition* definition = builder.getResult();

    if (definition == nullptr) {
        throw std::runtime_error("University complaint workflow definition could not be created");
    }

    TIVIDYFacade facade;

    WorkflowInstance* instance =facade.startWorkflow(definition);

    ProgressObserver progress;
    instance->attach(&progress);

    instance->startProcessing();

    std::cout<< "\nCase: Student U-2081 disputes a recorded mark.\n";
    std::cout<< "The complaint is registered, validated and investigated.\n\n";

    // Work completed before the routing decision.
    std::vector<WorkItem*> initialTasks;
    initialTasks.push_back(&recordComplaint);
    initialTasks.push_back(&validateComplaint);
    initialTasks.push_back(&investigateComplaint);

    for (std::vector<WorkItem*>::iterator it = initialTasks.begin();it != initialTasks.end();++it) {

        WorkItem* task = *it;

        Participant* assignee =facade.assignWork(instance,task,staff);

        if (assignee == nullptr) {
            instance->detach(&progress);
            delete instance;
            delete definition;

            throw std::runtime_error(
                "No staff member found for role or task dependency "
                "is incomplete: "
                + task->getRequiredRole()
            );
        }

        std::cout << YELLOW<< "[IN PROGRESS] "<< task->getName()<< " -> "<< assignee->getName()<< RESET<< "\n";
        facade.completeWork(instance,task,assignee);
        std::cout << GREEN << "[COMPLETED]   "<< task->getName()<< RESET<< "\n";
    }

    // Decision branch.
    char escalationChoice;

    std::cout
        << "\nDoes this complaint require senior review? (y/n): ";

    std::cin >> escalationChoice;

    bool requiresEscalation =escalationChoice == 'y' ||escalationChoice == 'Y';

    if (requiresEscalation) {

        std::cout << RED<< "\nDECISION: Senior review is required.\n"<< "EXCEPTION PATH: Escalating through the "<< "Chain of Responsibility...\n"<< RESET;

        facade.escalateWork(instance);

        std::cout << RED << "Escalation request passed through "<< "ApprovalHandler, EscalationHandler and "<< "FinalDecisionHandler.\n\n"<< RESET;
    }
    else {

        std::cout << GREEN<< "\nDECISION: Senior review is not required.\n"<< "NORMAL PATH: Continue directly to outcome review.\n\n"<< RESET;
    }

    // Both branches eventually return to the normal completion path.
    std::vector<WorkItem*> finalTasks;
    finalTasks.push_back(&reviewOutcome);
    finalTasks.push_back(&notifyStudent);
    finalTasks.push_back(&closeCase);

    for (std::vector<WorkItem*>::iterator it = finalTasks.begin();it != finalTasks.end();++it) {

        WorkItem* task = *it;

        Participant* assignee =
            facade.assignWork(instance,task,staff);

        if (assignee == nullptr) {
            instance->detach(&progress);
            delete instance;
            delete definition;

            throw std::runtime_error(
                "No staff member found for role or task dependency "
                "is incomplete: "
                + task->getRequiredRole()
            );
        }

        std::cout << YELLOW<< "[IN PROGRESS] " << task->getName()<< " -> "<< assignee->getName()<< RESET<< "\n";

        facade.completeWork(instance,task,assignee);

        std::cout << GREEN << "[COMPLETED]   " << task->getName()<< RESET<< "\n";
    }

    if (allWorkItemsCompleted(instance)) {

        std::cout << GREEN << BOLD << "\nRESULT: Complaint workflow resolved successfully.\n"<< RESET;
    }
    else {

        std::cout << RED<< "\nRESULT: Complaint remains open because "<< "work is outstanding.\n"<< RESET;
    }

    instance->detach(&progress);

    delete instance;
    delete definition;
}

void runUniversityComplaintInteractiveScenario()
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
