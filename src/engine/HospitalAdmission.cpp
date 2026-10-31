#include <iostream>
#include <stdexcept>
#include <vector>

#include "engine/WorkflowDefinition.h"
#include "engine/WorkflowInstance.h"
#include "engine/WorkItem.h"
#include "engine/Participant.h"
#include "engine/RoleBasedAssignment.h"
#include "engine/ProgressObserver.h"
#include "engine/TIVIDYFacade.h"
#include "engine/Console.h"
#include "engine/ConcreteWorkflowBuilder.h"

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

void runHospitalAdmissionScenario()
{
    using namespace tividy;
    using namespace tividy::console;

    std::cout << CYAN << BOLD<< "\n========================================\n";
    std::cout << " HOSPITAL: PATIENT ADMISSION WORKFLOW\n";
    std::cout << "========================================\n"<< RESET;

    WorkItem registerPatient(
        "Register patient",
        "Receptionist"
    );

    WorkItem verifyDocuments(
        "Verify admission documents",
        "Admissions Clerk"
    );

    WorkItem verifyPayment(
        "Verify medical aid/payment",
        "Billing Officer"
    );

    WorkItem emergencyReview(
        "Emergency clinical review",
        "Admissions Manager"
    );

    WorkItem approveAdmission(
        "Approve admission",
        "Admissions Manager"
    );

    WorkItem allocateBed(
        "Complete admission and allocate bed",
        "Ward Coordinator"
    );

    Participant receptionist("Nandi - Reception");
    receptionist.addRole("Receptionist");

    Participant admissionsClerk("Peter - Admissions");
    admissionsClerk.addRole("Admissions Clerk");

    Participant billingOfficer("Ayesha - Billing");
    billingOfficer.addRole("Billing Officer");

    Participant admissionsManager(
        "Dr Mokoena - Admissions Manager"
    );
    admissionsManager.addRole("Admissions Manager");

    Participant wardCoordinator(
        "Thabo - Ward Coordinator"
    );
    wardCoordinator.addRole("Ward Coordinator");

    std::vector<Participant*> staff;
    staff.push_back(&receptionist);
    staff.push_back(&admissionsClerk);
    staff.push_back(&billingOfficer);
    staff.push_back(&admissionsManager);
    staff.push_back(&wardCoordinator);

    // Decision input.
    char urgentChoice;

    std::cout
        << "\nIs this patient an urgent admission? (y/n): ";

    std::cin >> urgentChoice;

    bool urgentPatient =
        urgentChoice == 'y' ||
        urgentChoice == 'Y';

    RoleBasedAssignment roleAssignment;

    ConcreteWorkflowBuilder builder;

    // Common dependency path.
    builder.addDependency(
        &registerPatient,
        &verifyDocuments
    );

    builder.addDependency(
        &verifyDocuments,
        &verifyPayment
    );

    /*
     * Decision branch changes the actual workflow structure.
     *
     * Urgent:
     * verifyPayment -> emergencyReview -> approveAdmission
     *
     * Normal:
     * verifyPayment -> approveAdmission
     */
    if (urgentPatient) {
        builder.addDependency(&verifyPayment,&emergencyReview);
        builder.addDependency(&emergencyReview,&approveAdmission);
    }
    else {
        builder.addDependency(&verifyPayment,&approveAdmission);
    }

    builder.addDependency(&approveAdmission,&allocateBed);
    builder.setAssignmentStrategy(&roleAssignment);

    WorkflowDefinition* definition =
        builder.getResult();

    if (definition == nullptr) {
        throw std::runtime_error("Hospital workflow definition could not be created");
    }

    TIVIDYFacade facade;

    WorkflowInstance* instance =facade.startWorkflow(definition);

    ProgressObserver progress;
    instance->attach(&progress);

    instance->startProcessing();

    std::cout<< "\nCase: Patient P-1042 requires admission.\n\n";

    std::vector<WorkItem*> initialTasks;
    initialTasks.push_back(&registerPatient);
    initialTasks.push_back(&verifyDocuments);
    initialTasks.push_back(&verifyPayment);

    for (std::vector<WorkItem*>::iterator it = initialTasks.begin();it != initialTasks.end();++it) {

        WorkItem* task = *it;

        Participant* assignee =facade.assignWork(instance,task,staff);

        if (assignee == nullptr) {
            instance->detach(&progress);
            delete instance;
            delete definition;

            throw std::runtime_error(
                "No staff member found for role or task dependency " 
                "is incomplete: "+ task->getRequiredRole()
            );
        }

        std::cout << YELLOW<< "[IN PROGRESS] "<< task->getName()<< " -> "<< assignee->getName()<< RESET<< "\n";

        facade.completeWork(instance,task,assignee);

        std::cout << GREEN<< "[COMPLETED]   "<< task->getName()<< RESET<< "\n";
    }

    if (urgentPatient) {

        std::cout << RED<< "\nDECISION: Patient requires urgent review.\n"<< "EXCEPTION PATH: Emergency clinical review "<< "must occur before admission approval.\n"<< RESET;

        Participant* emergencyAssignee =
            facade.assignWork(instance,&emergencyReview,staff);

        if (emergencyAssignee == nullptr) {
            instance->detach(&progress);
            delete instance;
            delete definition;

            throw std::runtime_error(
                "Emergency clinical review could not be assigned"
            );
        }

        std::cout << YELLOW<< "[IN PROGRESS] "<< emergencyReview.getName()<< " -> "<< emergencyAssignee->getName()<< RESET<< "\n";

        facade.completeWork(
            instance,
            &emergencyReview,
            emergencyAssignee
        );

        std::cout << GREEN<< "[COMPLETED]   "<< emergencyReview.getName()<< RESET<< "\n";
    }
    else {

        std::cout << GREEN << "\nDECISION: Patient does not require urgent review.\n" << "NORMAL PATH: Continue directly to admission approval.\n"<< RESET;
    }

    std::vector<WorkItem*> finalTasks;
    finalTasks.push_back(&approveAdmission);
    finalTasks.push_back(&allocateBed);

    for (std::vector<WorkItem*>::iterator it = finalTasks.begin();it != finalTasks.end();++it) {

        WorkItem* task = *it;

        Participant* assignee =
            facade.assignWork(instance,task,staff );

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

        std::cout << YELLOW<< "[IN PROGRESS] " << task->getName()<< " -> " << assignee->getName() << RESET<< "\n";

        facade.completeWork( instance,task,assignee);

        std::cout << GREEN << "[COMPLETED]   "<< task->getName()<< RESET << "\n";
    }

    if (allWorkItemsCompleted(instance)) {

        std::cout << GREEN << BOLD << "\nRESULT: Patient admission workflow "<< "resolved successfully.\n"<< RESET;
    }
    else {

        std::cout << RED << "\nRESULT: Admission still has outstanding work.\n"<< RESET;
    }

    instance->detach(&progress);

    delete instance;
    delete definition;
}