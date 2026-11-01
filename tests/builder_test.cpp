#include "doctest.h"

#include <stdexcept>
#include <vector>

#include "engine/ConcreteWorkflowBuilder.h"
#include "engine/WorkflowBuilder.h"
#include "engine/WorkflowDefinition.h"
#include "engine/WorkflowDirector.h"
#include "engine/WorkflowStage.h"
#include "engine/WorkItem.h"
#include "engine/Role.h"
#include "engine/RoleBasedAssignment.h"
#include "engine/ApprovalHandler.h"

using namespace tividy;

TEST_CASE("ConcreteWorkflowBuilder can be used through WorkflowBuilder")
{
    WorkflowBuilder* builder = new ConcreteWorkflowBuilder();

    REQUIRE(builder != nullptr);

    delete builder;
}

TEST_CASE("Builder adds work items to the workflow definition")
{
    ConcreteWorkflowBuilder builder;

    WorkItem first("First task");
    WorkItem second("Second task");

    builder.addWorkItem(&first);
    builder.addWorkItem(&second);

    WorkflowDefinition* definition = builder.getResult();

    REQUIRE(definition != nullptr);
    REQUIRE(definition->getRoot() != nullptr);

    std::vector<WorkItem*> items;
    definition->getRoot()->collectWorkItems(items);

    REQUIRE(items.size() == 2);

    CHECK(items[0] == &first);
    CHECK(items[1] == &second);

    delete definition;
}

TEST_CASE("Builder records dependencies between work items")
{
    ConcreteWorkflowBuilder builder;

    WorkItem first("First task");
    WorkItem second("Second task");

    builder.addDependency(&first, &second);

    const std::vector<WorkItem*>& dependencies =
        second.getDependencies();

    REQUIRE(dependencies.size() == 1);
    CHECK(dependencies[0] == &first);

    delete builder.getResult();
}

TEST_CASE("Builder stores roles in the workflow definition")
{
    ConcreteWorkflowBuilder builder;

    Role<void> administrator("Administrator");

    builder.addRole(&administrator);

    WorkflowDefinition* definition = builder.getResult();

    REQUIRE(definition != nullptr);

    std::vector<Role<void>*> roles =
        definition->getRoles();

    REQUIRE(roles.size() == 1);
    CHECK(roles[0] == &administrator);

    delete definition;
}

TEST_CASE("Builder configures the assignment strategy")
{
    ConcreteWorkflowBuilder builder;

    RoleBasedAssignment strategy;

    builder.setAssignmentStrategy(&strategy);

    WorkflowDefinition* definition = builder.getResult();

    REQUIRE(definition != nullptr);

    CHECK(definition->getAssignmentStrategy() == &strategy);

    delete definition;
}

TEST_CASE("Builder configures the escalation handler")
{
    ConcreteWorkflowBuilder builder;

    ApprovalHandler handler;

    builder.setEscalationHandler(&handler);

    WorkflowDefinition* definition = builder.getResult();

    REQUIRE(definition != nullptr);

    WorkflowInstance instance(definition);

    CHECK_NOTHROW(
        definition->handleEscalation(&instance)
    );

    delete definition;
}

TEST_CASE("Builder rejects null work items")
{
    ConcreteWorkflowBuilder builder;

    CHECK_THROWS_AS(
        builder.addWorkItem(nullptr),
        std::invalid_argument
    );
}

TEST_CASE("Builder rejects null roles")
{
    ConcreteWorkflowBuilder builder;

    CHECK_THROWS_AS(
        builder.addRole(nullptr),
        std::invalid_argument
    );
}

TEST_CASE("Builder rejects invalid dependencies")
{
    ConcreteWorkflowBuilder builder;

    WorkItem item("Task");

    CHECK_THROWS_AS(
        builder.addDependency(nullptr, &item),
        std::invalid_argument
    );

    CHECK_THROWS_AS(
        builder.addDependency(&item, nullptr),
        std::invalid_argument
    );
}

TEST_CASE("Builder rejects negative priority")
{
    ConcreteWorkflowBuilder builder;

    WorkItem item("Urgent task");

    CHECK_THROWS_AS(
        builder.addPriority(&item, -1),
        std::invalid_argument
    );
}

TEST_CASE("getResult transfers the workflow definition")
{
    ConcreteWorkflowBuilder builder;

    WorkflowDefinition* first = builder.getResult();

    REQUIRE(first != nullptr);

    CHECK(builder.getResult() == nullptr);

    delete first;
}

TEST_CASE("WorkflowDirector rejects construction without a builder")
{
    WorkflowDirector director;

    CHECK_THROWS_AS(
        director.constructWorkflow(),
        std::logic_error
    );
}

TEST_CASE("WorkflowDirector accepts a configured builder")
{
    ConcreteWorkflowBuilder builder;

    WorkflowDirector director;
    director.setBuilder(&builder);

    CHECK_NOTHROW(
        director.constructWorkflow()
    );
}