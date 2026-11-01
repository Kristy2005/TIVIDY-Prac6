#include <iostream>
#include <stdexcept>
#include <vector>

#include "engine/WorkflowDefinition.h"
#include "engine/WorkflowInstance.h"
#include "engine/WorkItem.h"
#include "engine/Participant.h"
#include "engine/RoleBasedAssignment.h"
#include "engine/TIVIDYFacade.h"
#include "engine/Console.h"
#include "engine/ConcreteWorkflowBuilder.h"
#include "InteractiveWorkflowSession.h"

void runHospitalAdmissionScenario()
{
    using namespace tividy;
    using namespace tividy::console;

    std::cout << CYAN << BOLD << "\n========================================\n";
    std::cout << " HOSPITAL: PATIENT ADMISSION WORKFLOW\n";
    std::cout << "========================================\n" << RESET;

    WorkItem registerPatient("Register patient", "Receptionist");
    WorkItem verifyDocuments("Verify admission documents", "Admissions Clerk");
    WorkItem verifyPayment("Verify medical aid/payment", "Billing Officer");
    WorkItem emergencyReview("Emergency clinical review", "Admissions Manager");
    WorkItem approveAdmission("Approve admission", "Admissions Manager");
    WorkItem allocateBed("Complete admission and allocate bed", "Ward Coordinator");

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

    char urgentChoice = 'n';
    std::cout << "\nIs this patient an urgent admission? (y/n): ";
    if (!(std::cin >> urgentChoice)) {
        throw std::runtime_error("Could not read the urgent-admission decision");
    }
    const bool urgentPatient = urgentChoice == 'y' || urgentChoice == 'Y';

    RoleBasedAssignment roleAssignment;
    ConcreteWorkflowBuilder builder;

    builder.addDependency(&registerPatient, &verifyDocuments);
    builder.addDependency(&verifyDocuments, &verifyPayment);

    if (urgentPatient) {
        builder.addDependency(&verifyPayment, &emergencyReview);
        builder.addDependency(&emergencyReview, &approveAdmission);
    } else {
        builder.addDependency(&verifyPayment, &approveAdmission);
    }

    builder.addDependency(&approveAdmission, &allocateBed);
    builder.setAssignmentStrategy(&roleAssignment);

    WorkflowDefinition* definition = builder.getResult();
    if (definition == nullptr) {
        throw std::runtime_error("Hospital workflow definition could not be created");
    }

    TIVIDYFacade facade;
    WorkflowInstance* instance = nullptr;
    try {
        instance = facade.startWorkflow(definition);
        std::cout << "\nCase: Patient P-1042 requires admission.\n";
        if (urgentPatient) {
            std::cout << RED << "Decision: urgent admission. Emergency clinical review "
                      << "is a required dependency before approval.\n" << RESET;
        } else {
            std::cout << GREEN << "Decision: normal admission. The workflow proceeds "
                      << "directly from payment verification to approval.\n" << RESET;
        }

        runInteractiveWorkflowSession(
            facade,
            definition,
            instance,
            staff,
            "HOSPITAL - PATIENT P-1042"
        );
        instance = nullptr; // The interactive session owns and deletes it.
    } catch (...) {
        if (instance != nullptr) delete instance;
        delete definition;
        throw;
    }

    delete definition;
}
