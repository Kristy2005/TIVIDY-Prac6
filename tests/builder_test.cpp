// Builder pattern tests. Build and run from the engine folder:
//   g++ -std=c++11 -g -fsanitize=address,undefined -I. tests/builder_test.cpp $(ls *.cpp) -o builder_test && ./builder_test
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "ConcreteWorkflowBuilder.h"
#include "WorkflowDirector.h"
#include "WorkflowDefinition.h"
#include "WorkflowInstance.h"
#include "WorkflowStage.h"
#include "SubProcess.h"
#include "PriorityDecorator.h"
#include "RoleBasedAssignment.h"
#include "WorkloadBasedAssignment.h"
#include "TIVIDYFacade.h"

using namespace tividy;

static int checks = 0, failures = 0;
#define CHECK(cond) do { ++checks; if (!(cond)) { ++failures; \
    std::cout << "FAIL line " << __LINE__ << ": " #cond "\n"; } } while (0)
#define THROWS(expr, ExType) do { ++checks; bool caught = false; \
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
    CHECK(d != nullptr);
    CHECK(d->getName() == "Empty");
    CHECK(b.getResult() == nullptr);             // already transferred
    delete d;

    // Builder is reusable: a step after getResult() starts a new definition.
    ConcreteWorkflowBuilder r("Again");
    delete r.getResult();
    WorkItem* a = r.createWorkItem("A");
    WorkflowDefinition* d2 = r.getResult();
    CHECK(d2 != nullptr);
    CHECK(d2->getName() == "Again");
    CHECK(itemsOf(d2->getRoot()).size() == 1 && itemsOf(d2->getRoot())[0] == a);
    delete d2;

    // Unused builder frees its definition (checked by ASan).
    { ConcreteWorkflowBuilder unused; unused.createWorkItem("never collected"); }

    THROWS(ConcreteWorkflowBuilder(static_cast<WorkflowDefinition*>(nullptr)), std::invalid_argument);
}

static void testWorkItems() {
    ConcreteWorkflowBuilder b;
    THROWS(b.addWorkItem(nullptr), std::invalid_argument);
    THROWS(b.createWorkItem(""), std::invalid_argument);
    THROWS(b.createWorkItem("   "), std::invalid_argument);

    WorkItem callerOwned("Caller item", "Clerk");     // stack object: builder must not delete it
    b.addWorkItem(&callerOwned);
    b.addWorkItem(&callerOwned);                      // duplicate is ignored
    WorkItem* made = b.createWorkItem("Made item", "Doctor");
    CHECK(made->getName() == "Made item" && made->getRequiredRole() == "Doctor");

    WorkflowDefinition* d = b.getResult();
    std::vector<WorkItem*> items = itemsOf(d->getRoot());
    CHECK(items.size() == 2);
    CHECK(items[0] == &callerOwned && items[1] == made);

    WorkflowInstance* inst = d->createInstance();
    CHECK(inst->getWorkItemProgress().size() == 2);
    delete inst;
    delete d;                                          // must not touch callerOwned
}

static void testDependencies() {
    ConcreteWorkflowBuilder b;
    WorkItem* a = b.createWorkItem("A");
    WorkItem* c = b.createWorkItem("C");
    WorkItem* x = b.createWorkItem("X");

    THROWS(b.addDependency(nullptr, a), std::invalid_argument);
    THROWS(b.addDependency(a, nullptr), std::invalid_argument);
    THROWS(b.addDependency(a, a), std::invalid_argument);

    b.addDependency(a, c);                             // C waits for A
    b.addDependency(a, c);                             // repeat is harmless
    CHECK(c->getDependencies().size() == 1);
    b.addDependency(c, x);                             // X waits for C
    THROWS(b.addDependency(x, a), std::invalid_argument);   // cycle A->C->X->A
    THROWS(b.addDependency(c, a), std::invalid_argument);   // direct cycle
    CHECK(a->getDependencies().empty());               // failed calls changed nothing

    WorkItem outsider("Outsider");                     // not yet in tree: dependency adds it
    b.addDependency(&outsider, a);
    WorkflowDefinition* d = b.getResult();
    CHECK(itemsOf(d->getRoot()).size() == 4);

    WorkflowInstance* inst = d->createInstance();
    WorkItem* ia = inst->getInstanceWorkItem(a);
    WorkItem* ic = inst->getInstanceWorkItem(c);
    WorkItem* io = inst->getInstanceWorkItem(&outsider);
    CHECK(!inst->areDependenciesComplete(ic));
    CHECK(!inst->areDependenciesComplete(ia));
    inst->completeWorkItem(io);
    CHECK(inst->areDependenciesComplete(ia));
    inst->completeWorkItem(ia);
    CHECK(inst->areDependenciesComplete(ic));
    delete inst;
    delete d;
}

static void testPriority() {
    ConcreteWorkflowBuilder b;
    WorkItem* first = b.createWorkItem("First");
    WorkItem* second = b.createWorkItem("Second");
    WorkItem* third = b.createWorkItem("Third");
    THROWS(b.addPriority(nullptr, 1), std::invalid_argument);
    THROWS(b.addPriority(first, -1), std::invalid_argument);

    b.addPriority(second, 5);
    b.addPriority(second, 9);                          // update, not a second wrapper
    WorkItem late("Late");
    b.addPriority(&late, 3);                           // not yet in tree: added then wrapped
    b.addWorkItem(second);                             // already present via decorator: no duplicate
    CHECK(second->getPriority() == 9);
    CHECK(late.getPriority() == 3);

    WorkflowDefinition* d = b.getResult();
    std::vector<WorkItem*> items = itemsOf(d->getRoot());
    CHECK(items.size() == 4);
    CHECK(items[0] == first && items[1] == second && items[2] == third && items[3] == &late);  // order kept

    WorkflowStage* root = dynamic_cast<WorkflowStage*>(d->getRoot());
    CHECK(root != nullptr);
    WorkflowComponent* tree = d->createWorkTree();     // clone must not leak the wrapped clone (ASan)
    CHECK(itemsOf(tree).size() == 4);
    delete tree;

    WorkflowInstance* inst = d->createInstance();
    CHECK(inst->getInstanceWorkItem(second)->getPriority() == 9);
    CHECK(inst->getInstanceWorkItem(first)->getPriority() == 0);
    CHECK(inst->getWorkItemProgress().size() == 4);
    delete inst;
    delete d;
}

static void testRoles() {
    ConcreteWorkflowBuilder b;
    THROWS(b.addRole(nullptr), std::invalid_argument);
    THROWS(b.createRole(""), std::invalid_argument);

    Role<void> nurse("Nurse");
    b.addRole(&nurse);
    b.addRole(&nurse);                                 // same pointer: ignored
    Role<void> other("Nurse");
    THROWS(b.addRole(&other), std::invalid_argument);  // different object, same name

    Role<void>* doctor = b.createRole("Doctor");
    CHECK(b.createRole("Doctor") == doctor);           // find-or-create
    CHECK(b.createRole("Nurse") == &nurse);

    WorkflowDefinition* d = b.getResult();
    CHECK(d->getRoles().size() == 2);
    delete d;                                          // deletes doctor only
}

static void testParticipants() {
    ConcreteWorkflowBuilder b("Hospital");
    THROWS(b.addParticipant(nullptr), std::invalid_argument);
    THROWS(b.createParticipant(""), std::invalid_argument);
    THROWS(b.createParticipant("  "), std::invalid_argument);

    Participant alice("Alice");                        // caller-owned
    alice.addRole("Clerk");
    b.addParticipant(&alice);
    b.addParticipant(&alice);                          // same pointer: ignored
    Participant impostor("Alice");
    THROWS(b.addParticipant(&impostor), std::invalid_argument);
    Participant nameless;
    THROWS(b.addParticipant(&nameless), std::invalid_argument);

    Participant* bob = b.createParticipant("Bob", "Doctor");
    THROWS(b.createParticipant("Bob"), std::invalid_argument);
    Participant* carol = b.createParticipant("Carol");
    CHECK(bob->hasRole("Doctor") && carol->getRoles().empty());

    WorkItem* triage = b.createWorkItem("Triage", "Doctor");
    WorkItem* register_ = b.createWorkItem("Register", "Clerk");
    RoleBasedAssignment strategy;
    b.setAssignmentStrategy(&strategy);
    WorkflowDefinition* d = b.getResult();

    CHECK(d->getParticipants().size() == 3);
    CHECK(d->findParticipant("Bob") == bob);
    CHECK(d->findParticipant("Nobody") == nullptr);
    CHECK(d->getRoles().size() == 1);                  // "Doctor" auto-registered by createParticipant
    CHECK(d->getAssignmentStrategy() == &strategy);

    // Every instance of the definition starts with the definition's participants.
    WorkflowInstance* i1 = d->createInstance();
    WorkflowInstance* i2 = d->createInstance();
    CHECK(i1->getParticipantPointers().size() == 3);
    CHECK(i2->getParticipantPointers().size() == 3);
    CHECK(i1->getParticipantPointers()[1] == bob);

    // Workload is shared across instances of the same definition.
    i1->activate(); i2->activate();
    i1->assignParticipant(i1->getInstanceWorkItem(triage), bob);
    i2->assignParticipant(i2->getInstanceWorkItem(triage), bob);
    CHECK(bob->getWorkload() == 2);
    i1->completeWorkItem(i1->getInstanceWorkItem(triage));
    CHECK(bob->getWorkload() == 1);
    (void)register_;

    delete i1; delete i2;                              // instances first, then definition
    CHECK(bob->getWorkload() == 0);                    // destroying an instance releases its assignments
    delete d;
}

static void testSubProcess() {
    ConcreteWorkflowBuilder b;
    THROWS(b.endSubProcess(), std::logic_error);

    WorkItem* top1 = b.createWorkItem("Top1");
    b.beginSubProcess();
    WorkItem* in1 = b.createWorkItem("Inner1");
    b.beginSubProcess();
    WorkItem* deep = b.createWorkItem("Deep");
    THROWS(b.getResult(), std::logic_error);           // still open
    b.endSubProcess();
    WorkItem* in2 = b.createWorkItem("Inner2");
    b.endSubProcess();
    WorkItem* top2 = b.createWorkItem("Top2");
    b.addDependency(deep, top2);                       // across nesting levels, nothing moves
    THROWS(b.endSubProcess(), std::logic_error);

    WorkItem* top1Again = top1;
    b.addWorkItem(top1Again);                          // same place: fine
    b.beginSubProcess();
    THROWS(b.addWorkItem(top1), std::invalid_argument);// belongs to the root stage already
    b.endSubProcess();

    WorkflowDefinition* d = b.getResult();
    std::vector<WorkItem*> items = itemsOf(d->getRoot());
    CHECK(items.size() == 5);
    CHECK(items[0] == top1 && items[1] == in1 && items[2] == deep && items[3] == in2 && items[4] == top2);

    WorkflowStage* root = dynamic_cast<WorkflowStage*>(d->getRoot());
    CHECK(root != nullptr);
    WorkflowInstance* inst = d->createInstance();
    CHECK(inst->getWorkItemProgress().size() == 5);
    CHECK(!inst->areDependenciesComplete(inst->getInstanceWorkItem(top2)));
    delete inst;
    delete d;
}

static void testDirector() {
    WorkflowDirector director;
    THROWS(director.constructWorkflow(), std::logic_error);
    std::vector<WorkflowStep> steps;
    steps.push_back(WorkflowStep("Intake", "Clerk"));
    THROWS(director.constructSequentialWorkflow(steps), std::logic_error);
    THROWS(director.constructParallelWorkflow(steps), std::logic_error);

    ConcreteWorkflowBuilder b("Seq");
    director.setBuilder(&b);
    director.constructWorkflow();                      // no stored steps: no-op
    THROWS(director.addStep("", "Clerk"), std::invalid_argument);
    THROWS(director.addStep("Bad", "", -2), std::invalid_argument);

    director.addStep("Intake", "Clerk");
    director.addStep("Review", "Doctor", 4);
    director.addStep("Discharge", "Clerk", 2);
    director.constructWorkflow();
    WorkflowDefinition* d = b.getResult();
    std::vector<WorkItem*> items = itemsOf(d->getRoot());
    CHECK(items.size() == 3);
    CHECK(items[0]->getName() == "Intake" && items[0]->getRequiredRole() == "Clerk");
    CHECK(items[1]->getDependencies().size() == 1 && items[1]->getDependencies()[0] == items[0]);
    CHECK(items[2]->getDependencies()[0] == items[1]);
    CHECK(items[1]->getPriority() == 4 && items[2]->getPriority() == 2 && items[0]->getPriority() == 0);
    delete d;

    // Invalid recipe builds nothing.
    ConcreteWorkflowBuilder b2("Bad");
    director.setBuilder(&b2);
    std::vector<WorkflowStep> bad;
    bad.push_back(WorkflowStep("Fine"));
    bad.push_back(WorkflowStep(" "));
    THROWS(director.constructSequentialWorkflow(bad), std::invalid_argument);
    WorkflowDefinition* none = b2.getResult();
    CHECK(none->getRoot() == nullptr);
    delete none;

    // Parallel: no dependencies.
    ConcreteWorkflowBuilder b3("Par");
    director.setBuilder(&b3);
    std::vector<WorkflowStep> par;
    par.push_back(WorkflowStep("One")); par.push_back(WorkflowStep("Two", "R", 1));
    director.constructParallelWorkflow(par);
    WorkflowDefinition* pd = b3.getResult();
    std::vector<WorkItem*> pitems = itemsOf(pd->getRoot());
    CHECK(pitems.size() == 2 && pitems[0]->getDependencies().empty() && pitems[1]->getDependencies().empty());
    delete pd;
}

static void testFacade() {
    TIVIDYFacade facade;
    WorkflowDefinition* d = facade.createWorkflow("Complaints");
    CHECK(facade.getDefinitionName(d) == "Complaints");
    THROWS(facade.getDefinitionName(nullptr), std::invalid_argument);

    // Previously crashed: the facade's builder pointer was never initialised.
    WorkItem* log = facade.createWorkItemInDefinition(d, "Log complaint", "Clerk");
    WorkItem* investigate = facade.createWorkItemInDefinition(d, "Investigate", "Officer");
    WorkItem external("External step");
    facade.addWorkItemToDefinition(d, &external);
    facade.addDependencyToWorkItem(d, investigate, log);    // investigate waits for log
    facade.addPriorityToWorkItem(d, investigate, 7);
    facade.addPriorityToWorkItem(d, investigate, 8);        // twice: still one item
    THROWS(facade.addDependencyToWorkItem(d, log, investigate), std::invalid_argument);
    THROWS(facade.addWorkItemToDefinition(nullptr, log), std::invalid_argument);
    THROWS(facade.createWorkItemInDefinition(d, ""), std::invalid_argument);

    Participant* clerk = facade.createParticipantInDefinition(d, "Cleo", "Clerk");
    Participant* officer = facade.createParticipantInDefinition(d, "Omar", "Officer");
    Participant late("Lena");
    late.addRole("Clerk");
    facade.addParticipantToDefinition(d, &late);
    THROWS(facade.createParticipantInDefinition(d, "Cleo"), std::invalid_argument);
    THROWS(facade.addParticipantToDefinition(d, nullptr), std::invalid_argument);
    CHECK(facade.getParticipantsForDefinition(d).size() == 3);
    CHECK(facade.getAllParticipants().size() == 3);

    RoleBasedAssignment strategy;
    facade.configureAssignmentStrategy(d, &strategy);
    CHECK(d->getAssignmentStrategy() == &strategy);

    WorkflowInstance* inst = facade.startWorkflow(d);
    CHECK(facade.getParticipantObjects(inst).size() == 3);
    CHECK(facade.getAllWorkItems(inst).size() == 3);
    CHECK(inst->getInstanceWorkItem(investigate)->getPriority() == 8);

    // Participants picked up from the definition work with assignWork.
    std::vector<Participant*> pool = facade.getParticipantObjects(inst);
    CHECK(facade.assignWork(inst, log, pool) == clerk);
    CHECK(facade.assignWork(inst, investigate, pool) == nullptr);   // blocked until 'log' is done
    CHECK(clerk->getWorkload() == 1);
    (void)officer;

    // A second definition's builder is independent.
    WorkflowDefinition* d2 = facade.createWorkflow();
    facade.createWorkItemInDefinition(d2, "Solo");
    CHECK(itemsOf(d2->getRoot()).size() == 1);
    CHECK(itemsOf(d->getRoot()).size() == 3);

    // Director through the facade; bad recipe leaves nothing registered.
    std::size_t before = facade.getAllDefinitions().size();
    std::vector<WorkflowStep> steps;
    steps.push_back(WorkflowStep("S1", "A")); steps.push_back(WorkflowStep("S2", "B", 3));
    WorkflowDefinition* seq = facade.createSequentialWorkflow("Seq", steps);
    CHECK(facade.getAllDefinitions().size() == before + 1);
    CHECK(itemsOf(seq->getRoot()).size() == 2);
    std::vector<WorkflowStep> bad(1, WorkflowStep(""));
    THROWS(facade.createSequentialWorkflow("Bad", bad), std::invalid_argument);
    CHECK(facade.getAllDefinitions().size() == before + 1);

    // Instances before definitions.
    delete inst;
    delete d; delete d2; delete seq;
}

int main() {
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