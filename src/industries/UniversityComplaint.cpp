#include <iostream>
#include <stdexcept>
#include <vector>

#include "engine/WorkflowDefinition.h"
#include "engine/WorkflowInstance.h"
#include "engine/WorkflowStage.h"
#include "engine/WorkItem.h"
#include "engine/Participant.h"
#include "engine/RoleBasedAssignment.h"
#include "engine/ProgressObserver.h"
#include "engine/ApprovalHandler.h"
#include "engine/EscalationHandler.h"
#include "engine/FinalDecisionHandler.h"
#include "engine/Console.h"

void runUniversityComplaintScenario() {
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

    WorkflowStage complaintStage;
    complaintStage.add(&recordComplaint);
    complaintStage.add(&validateComplaint);
    complaintStage.add(&investigateComplaint);
    complaintStage.add(&reviewOutcome);
    complaintStage.add(&notifyStudent);
    complaintStage.add(&closeCase);

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

    WorkflowDefinition definition;
    definition.setRoot(&complaintStage);
    definition.setAssignmentStrategy(&roleAssignment);
    definition.setEscalationHandler(&initialReview);

    WorkflowInstance* instance = definition.createInstance();
    ProgressObserver progress;
    instance->attach(&progress);
    instance->activate();
    instance->startProcessing();

    std::vector<WorkItem*> tasks;
    tasks.push_back(&recordComplaint);
    tasks.push_back(&validateComplaint);
    tasks.push_back(&investigateComplaint);
    tasks.push_back(&reviewOutcome);
    tasks.push_back(&notifyStudent);
    tasks.push_back(&closeCase);

    std::cout << "\nCase: Student U-2081 disputes a recorded mark.\n";
    std::cout << "The complaint is registered, validated and investigated.\n\n";
    complaintStage.execute();

    for (std::size_t i = 0; i < 3; ++i) {
        WorkItem* task = tasks[i];
        Participant* assignee = definition.selectAssignee(task, staff);
        if (assignee == nullptr) {
            delete instance;
            throw std::runtime_error("No staff member found for role: " + task->getRequiredRole());
        }
        assignee->assignWork(task);
        std::cout << YELLOW << "[IN PROGRESS] " << task->getName()
                  << " -> " << assignee->getName() << RESET << "\n";
        assignee->completeWork(task);
        std::cout << GREEN << "[COMPLETED]   " << task->getName() << RESET << "\n";
    }

    std::cout << RED << "\nThe investigation requires a manager's decision.\n";
    std::cout << "Escalating through the configured Chain of Responsibility...\n" << RESET;
    instance->escalate();
    std::cout << RED << "Escalation request passed through ApprovalHandler, "
              << "EscalationHandler and FinalDecisionHandler.\n\n" << RESET;

    for (std::size_t i = 3; i < tasks.size(); ++i) {
        WorkItem* task = tasks[i];
        Participant* assignee = definition.selectAssignee(task, staff);
        if (assignee == nullptr) {
            delete instance;
            throw std::runtime_error("No staff member found for role: " + task->getRequiredRole());
        }
        assignee->assignWork(task);
        std::cout << YELLOW << "[IN PROGRESS] " << task->getName()
                  << " -> " << assignee->getName() << RESET << "\n";
        assignee->completeWork(task);
        std::cout << GREEN << "[COMPLETED]   " << task->getName() << RESET << "\n";
    }

    if (complaintStage.isComplete()) {
        instance->resolve();
        std::cout << GREEN << BOLD << "\nRESULT: Complaint workflow resolved; outcome notification sent "
                  << "as a simulated administrative step.\n" << RESET;
    } else {
        std::cout << RED << "\nRESULT: Complaint remains open because work is outstanding.\n" << RESET;
    }

    instance->detach(&progress);
    delete instance;
}