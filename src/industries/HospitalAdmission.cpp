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
#include "engine/TIVIDYFacade.h"
#include "engine/Console.h"

namespace {

bool allWorkItemsCompleted(const tividy::WorkflowInstance* instance) {
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

void runHospitalAdmissionScenario() {
    using namespace tividy;
    using namespace tividy::console;

    std::cout << CYAN << BOLD << "\n========================================\n";
    std::cout << " HOSPITAL: PATIENT ADMISSION WORKFLOW\n";
    std::cout << "========================================\n" << RESET;

    WorkItem registerPatient("Register patient", "Receptionist");
    WorkItem verifyDocuments("Verify admission documents", "Admissions Clerk");
    WorkItem verifyPayment("Verify medical aid/payment", "Billing Officer");
    WorkItem approveAdmission("Approve admission", "Admissions Manager");
    WorkItem allocateBed("Complete admission and allocate bed", "Ward Coordinator");

    WorkflowStage admissionStage;
    admissionStage.add(&registerPatient);
    admissionStage.add(&verifyDocuments);
    admissionStage.add(&verifyPayment);
    admissionStage.add(&approveAdmission);
    admissionStage.add(&allocateBed);

    Participant receptionist("Nandi - Reception");
    receptionist.addRole("Receptionist");
    Participant admissionsClerk("Peter - Admissions");
    admissionsClerk.addRole("Admissions Clerk");
    Participant billingOfficer("Ayesha - Billing");
    billingOfficer.addRole("Billing Officer");
    Participant admissionsManager("Dr Mokoena - Admissions Manager");
    admissionsManager.addRole("Admissions Manager");
    Participant wardCoordinator("Thabo - Ward Coordinator");
    wardCoordinator.addRole("Ward Coordinator");

    std::vector<Participant*> staff;
    staff.push_back(&receptionist);
    staff.push_back(&admissionsClerk);
    staff.push_back(&billingOfficer);
    staff.push_back(&admissionsManager);
    staff.push_back(&wardCoordinator);

    RoleBasedAssignment roleAssignment;
    WorkflowDefinition definition;
    definition.setRoot(&admissionStage);
    definition.setAssignmentStrategy(&roleAssignment);

    // Use the Facade so assignment and completion are recorded against this
    // particular workflow instance and participant workload is maintained.
    TIVIDYFacade facade;
    WorkflowInstance* instance = facade.startWorkflow(&definition);
    ProgressObserver progress;
    instance->attach(&progress);
    instance->startProcessing();

    std::vector<WorkItem*> tasks;
    tasks.push_back(&registerPatient);
    tasks.push_back(&verifyDocuments);
    tasks.push_back(&verifyPayment);
    tasks.push_back(&approveAdmission);
    tasks.push_back(&allocateBed);

    std::cout << "\nCase: Patient P-1042 requires a planned admission.\n";
    std::cout << "The workflow assigns and completes each task through the facade.\n\n";

    for (std::vector<WorkItem*>::iterator it = tasks.begin(); it != tasks.end(); ++it) {
        WorkItem* task = *it;
        Participant* assignee = facade.assignWork(instance, task, staff);

        if (assignee == nullptr) {
            instance->detach(&progress);
            delete instance;
            throw std::runtime_error(
                "No staff member found for role: " + task->getRequiredRole()
            );
        }

        std::cout << YELLOW << "[IN PROGRESS] " << task->getName()
                  << " -> " << assignee->getName() << RESET << "\n";

        facade.completeWork(instance, task, assignee);

        std::cout << GREEN << "[COMPLETED]   " << task->getName() << RESET << "\n";
    }

    if (allWorkItemsCompleted(instance)) {
        std::cout << GREEN << BOLD
                  << "\nRESULT: Patient admission workflow resolved successfully.\n"
                  << RESET;
    } else {
        std::cout << RED
                  << "\nRESULT: Admission still has outstanding work.\n"
                  << RESET;
    }

    instance->detach(&progress);
    delete instance;
}
