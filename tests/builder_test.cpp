#include "doctest.h"
#include "engine/WorkflowBuilder.h"

using namespace tividy;

namespace {
void declareRoles(WorkflowBuilder& b) { b.addRole("clerk"); b.addRole("reviewer"); }
}

TEST_CASE("Builder assembles a definition step by step") {
    ConcreteWorkflowBuilder b("Sample");
    declareRoles(b);
    b.addStage("s1", "Stage one");
    b.addWorkItem("s1", WorkItemSpec("intake", "Intake", "clerk"));
    b.addWorkItem("s1", WorkItemSpec("review", "Review", "reviewer", Priority::Urgent).after("intake"));
    b.addSubProcess("sub", "Nested", "s1");
    b.addWorkItem("sub", WorkItemSpec("check", "Check", "clerk").after("review"));
    b.addEscalationHandler(new WorkHandler("clerk", 1));
    b.addEscalationHandler(new FinalDecisionHandler("reviewer"));
    std::shared_ptr<WorkflowDefinition> def = b.build();

    CHECK(def->name() == "Sample");
    CHECK(def->hasRole("clerk"));
    CHECK(def->prototype().find("check") != 0);
    CHECK(def->prototype().find("review")->priority() == Priority::Urgent);
    REQUIRE(def->escalationChain() != 0);
    CHECK(def->escalationChain()->next() != 0);
    CHECK(def->strategy().name() == "RoleBased");  // default
}

TEST_CASE("Instances get independent copies of the work tree") {
    ConcreteWorkflowBuilder b("Sample");
    declareRoles(b);
    b.addWorkItem("", WorkItemSpec("only", "Only", "clerk"));
    std::shared_ptr<WorkflowDefinition> def = b.build();
    std::unique_ptr<WorkflowComponent> t1(def->createWorkTree());
    std::unique_ptr<WorkflowComponent> t2(def->createWorkTree());
    t1->find("only")->makeAvailable();
    CHECK(t2->find("only")->status() == WorkStatus::Pending);
    CHECK(def->prototype().find("only")->status() == WorkStatus::Pending);
}

TEST_CASE("Routing rules add a condition and its dependency") {
    ConcreteWorkflowBuilder b("Branching");
    declareRoles(b);
    b.addWorkItem("", WorkItemSpec("decide", "Decide", "reviewer"));
    b.addWorkItem("", WorkItemSpec("yes", "Yes path", "clerk"));
    b.addWorkItem("", WorkItemSpec("no", "No path", "clerk"));
    b.addRoutingRule("decide", "approved", "yes");
    b.addRoutingRule("decide", "rejected", "no");
    std::shared_ptr<WorkflowDefinition> def = b.build();
    CHECK(def->prototype().find("yes")->conditionOutcome() == "approved");
    CHECK(def->prototype().find("no")->dependencies().size() == 1);
}

TEST_CASE("Invalid definitions are rejected when built") {
    SUBCASE("empty workflow") {
        ConcreteWorkflowBuilder b("Empty");
        CHECK_THROWS_AS(b.build(), WorkflowError);
    }
    SUBCASE("undeclared role") {
        ConcreteWorkflowBuilder b("Bad");
        b.addWorkItem("", WorkItemSpec("x", "X", "ghost"));
        CHECK_THROWS_AS(b.build(), WorkflowError);
    }
    SUBCASE("dependency on a missing item") {
        ConcreteWorkflowBuilder b("Bad");
        declareRoles(b);
        b.addWorkItem("", WorkItemSpec("x", "X", "clerk").after("missing"));
        CHECK_THROWS_AS(b.build(), WorkflowError);
    }
    SUBCASE("circular dependency") {
        ConcreteWorkflowBuilder b("Cycle");
        declareRoles(b);
        b.addWorkItem("", WorkItemSpec("a", "A", "clerk").after("b"));
        b.addWorkItem("", WorkItemSpec("b", "B", "clerk").after("a"));
        CHECK_THROWS_AS(b.build(), WorkflowError);
    }
    SUBCASE("duplicate ids") {
        ConcreteWorkflowBuilder b("Dup");
        declareRoles(b);
        b.addWorkItem("", WorkItemSpec("a", "A", "clerk"));
        CHECK_THROWS_AS(b.addWorkItem("", WorkItemSpec("a", "A again", "clerk")), WorkflowError);
    }
}

TEST_CASE("Director builds a sequential process through the Builder interface") {
    ConcreteWorkflowBuilder b;
    std::vector<WorkItemSpec> steps;
    steps.push_back(WorkItemSpec("one", "One", "clerk"));
    steps.push_back(WorkItemSpec("two", "Two", "reviewer"));
    steps.push_back(WorkItemSpec("three", "Three", "clerk"));
    std::shared_ptr<WorkflowDefinition> def = WorkflowDirector::buildSequential(b, "Linear", steps);
    CHECK(def->prototype().find("two")->dependencies()[0] == "one");
    CHECK(def->prototype().find("three")->dependencies()[0] == "two");
    CHECK(def->roles().size() == 2);
}
