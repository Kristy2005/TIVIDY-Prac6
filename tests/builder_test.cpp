// Builder pattern tests. Build and run from the engine folder:
//   g++ -std=c++11 -g -fsanitize=address,undefined -I. tests/builder_test.cpp $(ls *.cpp) -o builder_test && ./builder_test
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "engine/ConcreteWorkflowBuilder.h"
#include "engine/WorkflowDirector.h"
#include "engine/WorkflowDefinition.h"
#include "engine/WorkflowInstance.h"
#include "engine/WorkflowStage.h"
#include "engine/SubProcess.h"
#include "engine/PriorityDecorator.h"
#include "engine/RoleBasedAssignment.h"
#include "engine/WorkloadBasedAssignment.h"
#include "engine/TIVIDYFacade.h"

using namespace tividy;

static int checks = 0, failures = 0;
#define BCHECK(cond) do { ++checks; if (!(cond)) { ++failures; \
    std::cout << "FAIL line " << __LINE__ << ": " #cond "\n"; } } while (0)
#define BTHROWS(expr, ExType) do { ++checks; bool caught = false; \
    try { expr; } catch (const ExType&) { caught = true; } catch (...) {} \
    if (!caught) { ++failures; std::cout << "FAIL line " << __LINE__ \
        << ": expected " #ExType " from " #expr "\n"; } } while (0)

static std::vector<WorkItem*> itemsOf(WorkflowComponent* root) {
    std::vector<WorkItem*> items;
    if (root) root->collectWorkItems(items);
    return items;
}

static void testLifecycleAndReuse() {
    ConcreteWorkflowBuilder b("Empty");
    WorkflowDefinition* d = b.getResult();
    BCHECK(d != nullptr);
    BCHECK(d->getName() == "Empty");
    BCHECK(b.getResult() == nullptr);             // already transferred
    delete d;

    // Builder is reusable: a step after getResult() starts a new definition.
    ConcreteWorkflowBuilder r("Again");
    delete r.getResult();
    WorkItem* a = r.createWorkItem("A");
    WorkflowDefinition* d2 = r.getResult();
    BCHECK(d2 != nullptr);
    BCHECK(d2->getName() == "Again");
    BCHECK(itemsOf(d2->getRoot()).size() == 1 && itemsOf(d2->getRoot())[0] == a);
    delete d2;

    // Unused builder frees its definition (checked by ASan).
    { ConcreteWorkflowBuilder unused; unused.createWorkItem("never collected"); }

    BTHROWS(ConcreteWorkflowBuilder(static_cast<WorkflowDefinition*>(nullptr)), std::invalid_argument);
}

static void testWorkItems() {
    ConcreteWorkflowBuilder b;
    BTHROWS(b.addWorkItem(nullptr), std::invalid_argument);
    BTHROWS(b.createWorkItem(""), std::invalid_argument);
    BTHROWS(b.createWorkItem("   "), std::invalid_argument);

    WorkItem callerOwned("Caller item", "Clerk");     // stack object: builder must not delete it
    b.addWorkItem(&callerOwned);
    b.addWorkItem(&callerOwned);                      // duplicate is ignored
    WorkItem* made = b.createWorkItem("Made item", "Doctor");
    BCHECK(made->getName() == "Made item" && made->getRequiredRole() == "Doctor");

    WorkflowDefinition* d = b.getResult();
    std::vector<WorkItem*> items = itemsOf(d->getRoot());
    BCHECK(items.size() == 2);
    BCHECK(items[0] == &callerOwned && items[1] == made);

    WorkflowInstance* inst = d->createInstance();
    BCHECK(inst->getWorkItemProgress().size() == 2);
    delete inst;
    delete d;                                          // must not touch callerOwned
}

static void testDependencies() {
    ConcreteWorkflowBuilder b;
    WorkItem* a = b.createWorkItem("A");
    WorkItem* c = b.createWorkItem("C");
    WorkItem* x = b.createWorkItem("X");

    BTHROWS(b.addDependency(nullptr, a), std::invalid_argument);
    BTHROWS(b.addDependency(a, nullptr), std::invalid_argument);
    BTHROWS(b.addDependency(a, a), std::invalid_argument);

    b.addDependency(a, c);                             // C waits for A
    b.addDependency(a, c);                             // repeat is harmless
    BCHECK(c->getDependencies().size() == 1);
    b.addDependency(c, x);                             // X waits for C
    BTHROWS(b.addDependency(x, a), std::invalid_argument);   // cycle A->C->X->A
    BTHROWS(b.addDependency(c, a), std::invalid_argument);   // direct cycle
    BCHECK(a->getDependencies().empty());               // failed calls changed nothing

    WorkItem outsider("Outsider");                     // not yet in tree: dependency adds it
    b.addDependency(&outsider, a);
    WorkflowDefinition* d = b.getResult();
    BCHECK(itemsOf(d->getRoot()).size() == 4);

    WorkflowInstance* inst = d->createInstance();
    WorkItem* ia = inst->getInstanceWorkItem(a);
    WorkItem* ic = inst->getInstanceWorkItem(c);
    WorkItem* io = inst->getInstanceWorkItem(&outsider);
    BCHECK(!inst->areDependenciesComplete(ic));
    BCHECK(!inst->areDependenciesComplete(ia));
    inst->completeWorkItem(io);
    BCHECK(inst->areDependenciesComplete(ia));
    inst->completeWorkItem(ia);
    BCHECK(inst->areDependenciesComplete(ic));
    delete inst;
    delete d;
}

static void testPriority() {
    ConcreteWorkflowBuilder b;
    WorkItem* first = b.createWorkItem("First");
    WorkItem* second = b.createWorkItem("Second");
    WorkItem* third = b.createWorkItem("Third");
    BTHROWS(b.addPriority(nullptr, 1), std::invalid_argument);
    BTHROWS(b.addPriority(first, -1), std::invalid_argument);

    b.addPriority(second, 5);
    b.addPriority(second, 9);                          // update, not a second wrapper
    WorkItem late("Late");
    b.addPriority(&late, 3);                           // not yet in tree: added then wrapped
    b.addWorkItem(second);                             // already present via decorator: no duplicate
    BCHECK(second->getPriority() == 9);
    BCHECK(late.getPriority() == 3);

    WorkflowDefinition* d = b.getResult();
    std::vector<WorkItem*> items = itemsOf(d->getRoot());
    BCHECK(items.size() == 4);
    BCHECK(items[0] == first && items[1] == second && items[2] == third && items[3] == &late);  // order kept

    WorkflowStage* root = dynamic_cast<WorkflowStage*>(d->getRoot());
    BCHECK(root != nullptr);
    WorkflowComponent* tree = d->createWorkTree();     // clone must not leak the wrapped clone (ASan)
    BCHECK(itemsOf(tree).size() == 4);
    delete tree;

    WorkflowInstance* inst = d->createInstance();
    BCHECK(inst->getInstanceWorkItem(second)->getPriority() == 9);
    BCHECK(inst->getInstanceWorkItem(first)->getPriority() == 0);
    BCHECK(inst->getWorkItemProgress().size() == 4);
    delete inst;
    delete d;
}

static void testRoles() {
    ConcreteWorkflowBuilder b;
    BTHROWS(b.addRole(nullptr), std::invalid_argument);
    BTHROWS(b.createRole(""), std::invalid_argument);

    Role<void> nurse("Nurse");
    b.addRole(&nurse);
    b.addRole(&nurse);                                 // same pointer: ignored
    Role<void> other("Nurse");
    BTHROWS(b.addRole(&other), std::invalid_argument);  // different object, same name

    Role<void>* doctor = b.createRole("Doctor");
    BCHECK(b.createRole("Doctor") == doctor);           // find-or-create
    BCHECK(b.createRole("Nurse") == &nurse);

    WorkflowDefinition* d = b.getResult();
    BCHECK(d->getRoles().size() == 2);
    delete d;                                          // deletes doctor only
}

static void testParticipants() {
    ConcreteWorkflowBuilder b("Hospital");
    BTHROWS(b.addParticipant(nullptr), std::invalid_argument);
    BTHROWS(b.createParticipant(""), std::invalid_argument);
    BTHROWS(b.createParticipant("  "), std::invalid_argument);

    Participant alice("Alice");                        // caller-owned
    alice.addRole("Clerk");
    b.addParticipant(&alice);
    b.addParticipant(&alice);                          // same pointer: ignored
    Participant impostor("Alice");
    BTHROWS(b.addParticipant(&impostor), std::invalid_argument);
    Participant nameless;
    BTHROWS(b.addParticipant(&nameless), std::invalid_argument);

    Participant* bob = b.createParticipant("Bob", "Doctor");
    BTHROWS(b.createParticipant("Bob"), std::invalid_argument);
    Participant* carol = b.createParticipant("Carol");
    BCHECK(bob->hasRole("Doctor") && carol->getRoles().empty());

    WorkItem* triage = b.createWorkItem("Triage", "Doctor");
    WorkItem* register_ = b.createWorkItem("Register", "Clerk");
    RoleBasedAssignment strategy;
    b.setAssignmentStrategy(&strategy);
    WorkflowDefinition* d = b.getResult();

    BCHECK(d->getParticipants().size() == 3);
    BCHECK(d->findParticipant("Bob") == bob);
    BCHECK(d->findParticipant("Nobody") == nullptr);
    BCHECK(d->getRoles().size() == 1);                  // "Doctor" auto-registered by createParticipant
    BCHECK(d->getAssignmentStrategy() == &strategy);

    // Every instance of the definition starts with the definition's participants.
    WorkflowInstance* i1 = d->createInstance();
    WorkflowInstance* i2 = d->createInstance();
    BCHECK(i1->getParticipantPointers().size() == 3);
    BCHECK(i2->getParticipantPointers().size() == 3);
    BCHECK(i1->getParticipantPointers()[1] == bob);

    // Workload is shared across instances of the same definition.
    i1->activate(); i2->activate();
    i1->assignParticipant(i1->getInstanceWorkItem(triage), bob);
    i2->assignParticipant(i2->getInstanceWorkItem(triage), bob);
    BCHECK(bob->getWorkload() == 2);
    i1->completeWorkItem(i1->getInstanceWorkItem(triage));
    BCHECK(bob->getWorkload() == 1);
    (void)register_;

    delete i1; delete i2;                              // instances first, then definition
    BCHECK(bob->getWorkload() == 0);                    // destroying an instance releases its assignments
    delete d;
}

static void testSubProcess() {
    ConcreteWorkflowBuilder b;
    BTHROWS(b.endSubProcess(), std::logic_error);

    WorkItem* top1 = b.createWorkItem("Top1");
    b.beginSubProcess();
    WorkItem* in1 = b.createWorkItem("Inner1");
    b.beginSubProcess();
    WorkItem* deep = b.createWorkItem("Deep");
    BTHROWS(b.getResult(), std::logic_error);           // still open
    b.endSubProcess();
    WorkItem* in2 = b.createWorkItem("Inner2");
    b.endSubProcess();
    WorkItem* top2 = b.createWorkItem("Top2");
    b.addDependency(deep, top2);                       // across nesting levels, nothing moves
    BTHROWS(b.endSubProcess(), std::logic_error);

    WorkItem* top1Again = top1;
    b.addWorkItem(top1Again);                          // same place: fine
    b.beginSubProcess();
    BTHROWS(b.addWorkItem(top1), std::invalid_argument);// belongs to the root stage already
    b.endSubProcess();

    WorkflowDefinition* d = b.getResult();
    std::vector<WorkItem*> items = itemsOf(d->getRoot());
    BCHECK(items.size() == 5);
    BCHECK(items[0] == top1 && items[1] == in1 && items[2] == deep && items[3] == in2 && items[4] == top2);

    WorkflowStage* root = dynamic_cast<WorkflowStage*>(d->getRoot());
    BCHECK(root != nullptr);
    WorkflowInstance* inst = d->createInstance();
    BCHECK(inst->getWorkItemProgress().size() == 5);
    BCHECK(!inst->areDependenciesComplete(inst->getInstanceWorkItem(top2)));
    delete inst;
    delete d;
}

static void testDirector() {
    WorkflowDirector director;
    BTHROWS(director.constructWorkflow(), std::logic_error);
    std::vector<WorkflowStep> steps;
    steps.push_back(WorkflowStep("Intake", "Clerk"));
    BTHROWS(director.constructSequentialWorkflow(steps), std::logic_error);
    BTHROWS(director.constructParallelWorkflow(steps), std::logic_error);

    ConcreteWorkflowBuilder b("Seq");
    director.setBuilder(&b);
    director.constructWorkflow();                      // no stored steps: no-op
    BTHROWS(director.addStep("", "Clerk"), std::invalid_argument);
    BTHROWS(director.addStep("Bad", "", -2), std::invalid_argument);

    director.addStep("Intake", "Clerk");
    director.addStep("Review", "Doctor", 4);
    director.addStep("Discharge", "Clerk", 2);
    director.constructWorkflow();
    WorkflowDefinition* d = b.getResult();
    std::vector<WorkItem*> items = itemsOf(d->getRoot());
    BCHECK(items.size() == 3);
    BCHECK(items[0]->getName() == "Intake" && items[0]->getRequiredRole() == "Clerk");
    BCHECK(items[1]->getDependencies().size() == 1 && items[1]->getDependencies()[0] == items[0]);
    BCHECK(items[2]->getDependencies()[0] == items[1]);
    BCHECK(items[1]->getPriority() == 4 && items[2]->getPriority() == 2 && items[0]->getPriority() == 0);
    delete d;

    // Invalid recipe builds nothing.
    ConcreteWorkflowBuilder b2("Bad");
    director.setBuilder(&b2);
    std::vector<WorkflowStep> bad;
    bad.push_back(WorkflowStep("Fine"));
    bad.push_back(WorkflowStep(" "));
    BTHROWS(director.constructSequentialWorkflow(bad), std::invalid_argument);
    WorkflowDefinition* none = b2.getResult();
    BCHECK(none->getRoot() == nullptr);
    delete none;

    // Parallel: no dependencies.
    ConcreteWorkflowBuilder b3("Par");
    director.setBuilder(&b3);
    std::vector<WorkflowStep> par;
    par.push_back(WorkflowStep("One")); par.push_back(WorkflowStep("Two", "R", 1));
    director.constructParallelWorkflow(par);
    WorkflowDefinition* pd = b3.getResult();
    std::vector<WorkItem*> pitems = itemsOf(pd->getRoot());
    BCHECK(pitems.size() == 2 && pitems[0]->getDependencies().empty() && pitems[1]->getDependencies().empty());
    delete pd;
}

static void testFacade() {
    TIVIDYFacade facade;
    WorkflowDefinition* d = facade.createWorkflow("Complaints");
    BCHECK(facade.getDefinitionName(d) == "Complaints");
    BTHROWS(facade.getDefinitionName(nullptr), std::invalid_argument);

    // Previously crashed: the facade's builder pointer was never initialised.
    WorkItem* log = facade.createWorkItemInDefinition(d, "Log complaint", "Clerk");
    WorkItem* investigate = facade.createWorkItemInDefinition(d, "Investigate", "Officer");
    WorkItem external("External step");
    facade.addWorkItemToDefinition(d, &external);
    facade.addDependencyToWorkItem(d, investigate, log);    // investigate waits for log
    facade.addPriorityToWorkItem(d, investigate, 7);
    facade.addPriorityToWorkItem(d, investigate, 8);        // twice: still one item
    BTHROWS(facade.addDependencyToWorkItem(d, log, investigate), std::invalid_argument);
    BTHROWS(facade.addWorkItemToDefinition(nullptr, log), std::invalid_argument);
    BTHROWS(facade.createWorkItemInDefinition(d, ""), std::invalid_argument);

    Participant* clerk = facade.createParticipantInDefinition(d, "Cleo", "Clerk");
    Participant* officer = facade.createParticipantInDefinition(d, "Omar", "Officer");
    Participant late("Lena");
    late.addRole("Clerk");
    facade.addParticipantToDefinition(d, &late);
    BTHROWS(facade.createParticipantInDefinition(d, "Cleo"), std::invalid_argument);
    BTHROWS(facade.addParticipantToDefinition(d, nullptr), std::invalid_argument);
    BCHECK(facade.getParticipantsForDefinition(d).size() == 3);
    BCHECK(facade.getAllParticipants().size() == 3);

    RoleBasedAssignment strategy;
    facade.configureAssignmentStrategy(d, &strategy);
    BCHECK(d->getAssignmentStrategy() == &strategy);

    WorkflowInstance* inst = facade.startWorkflow(d);
    BCHECK(facade.getParticipantObjects(inst).size() == 3);
    BCHECK(facade.getAllWorkItems(inst).size() == 3);
    BCHECK(inst->getInstanceWorkItem(investigate)->getPriority() == 8);

    // Participants picked up from the definition work with assignWork.
    std::vector<Participant*> pool = facade.getParticipantObjects(inst);
    BCHECK(facade.assignWork(inst, log, pool) == clerk);
    BCHECK(facade.assignWork(inst, investigate, pool) == nullptr);   // blocked until 'log' is done
    BCHECK(clerk->getWorkload() == 1);
    (void)officer;

    // A second definition's builder is independent.
    WorkflowDefinition* d2 = facade.createWorkflow();
    facade.createWorkItemInDefinition(d2, "Solo");
    BCHECK(itemsOf(d2->getRoot()).size() == 1);
    BCHECK(itemsOf(d->getRoot()).size() == 3);

    // Director through the facade; bad recipe leaves nothing registered.
    std::size_t before = facade.getAllDefinitions().size();
    std::vector<WorkflowStep> steps;
    steps.push_back(WorkflowStep("S1", "A")); steps.push_back(WorkflowStep("S2", "B", 3));
    WorkflowDefinition* seq = facade.createSequentialWorkflow("Seq", steps);
    BCHECK(facade.getAllDefinitions().size() == before + 1);
    BCHECK(itemsOf(seq->getRoot()).size() == 2);
    std::vector<WorkflowStep> bad(1, WorkflowStep(""));
    BTHROWS(facade.createSequentialWorkflow("Bad", bad), std::invalid_argument);
    BCHECK(facade.getAllDefinitions().size() == before + 1);

    // Instances before definitions.
    delete inst;
    delete d; delete d2; delete seq;
}

int runBuilderChecks() {
    testLifecycleAndReuse();
    testWorkItems();
    testDependencies();
    testPriority();
    testRoles();
    testParticipants();
    testSubProcess();
    testDirector();
    testFacade();
    std::cout << checks << " checks, " << failures << " failures\n";
    return failures == 0 ? 0 : 1;
}
#include "doctest.h"

TEST_CASE("builder: full suite") {
    DOCTEST_CHECK(runBuilderChecks() == 0);
}
