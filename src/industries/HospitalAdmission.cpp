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

void runHospitalAdmissionScenario() {
    using namespace tividy;

    std::cout << "\n========================================\n";
    std::cout << " HOSPITAL: PATIENT ADMISSION WORKFLOW\n";
    std::cout << "========================================\n";

    // Work items are ordered by the WorkflowStage (Composite pattern).
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

    WorkflowInstance* instance = definition.createInstance();
    ProgressObserver progress;
    instance->attach(&progress);
    instance->activate();
    instance->startProcessing();

    std::vector<WorkItem*> tasks;
    tasks.push_back(&registerPatient);
    tasks.push_back(&verifyDocuments);
    tasks.push_back(&verifyPayment);
    tasks.push_back(&approveAdmission);
    tasks.push_back(&allocateBed);

    std::cout << "\nCase: Patient P-1042 requires a planned admission.\n";
    std::cout << "The engine executes the workflow stage in configured order.\n\n";
    admissionStage.execute();

    for (std::vector<WorkItem*>::iterator it = tasks.begin(); it != tasks.end(); ++it) {
        WorkItem* task = *it;
        Participant* assignee = definition.selectAssignee(task, staff);
        if (assignee == nullptr) {
            delete instance;
            throw std::runtime_error("No staff member found for role: " + task->getRequiredRole());
        }
        assignee->assignWork(task);
        std::cout << "[IN PROGRESS] " << task->getName()
                  << " -> " << assignee->getName() << "\n";
        assignee->completeWork(task);
        std::cout << "[COMPLETED]   " << task->getName() << "\n";
    }

    if (admissionStage.isComplete()) {
        instance->resolve();
        std::cout << "\nRESULT: Patient admission workflow resolved successfully.\n";
    } else {
        std::cout << "\nRESULT: Admission still has outstanding work.\n";
    }

    instance->detach(&progress);
    delete instance;
}
