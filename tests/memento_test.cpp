#include "doctest.h"

#include <stdexcept>

#include "engine/WorkflowDefinition.h"
#include "engine/WorkflowInstance.h"
#include "engine/WorkflowMemento.h"
#include "engine/SubmittedState.h"
#include "engine/ActiveState.h"
#include "engine/InProgressState.h"

using namespace tividy;

TEST_CASE("WorkflowMemento starts with no saved state")
{
    WorkflowMemento memento;

    CHECK(memento.getState() == nullptr);
}

TEST_CASE("WorkflowMemento stores a workflow state pointer")
{
    WorkflowMemento memento;

    static ActiveState state;

    memento.setState(&state);

    CHECK(memento.getState() == &state);
}

TEST_CASE("WorkflowInstance creates a memento of its current state")
{
    WorkflowDefinition definition;
    WorkflowInstance instance(&definition);

    WorkflowState* originalState = instance.getState();

    WorkflowMemento* memento = instance.createMemento();

    REQUIRE(memento != nullptr);
    CHECK(memento->getState() == originalState);

    delete memento;
}

TEST_CASE("WorkflowInstance restores the state stored in a memento")
{
    WorkflowDefinition definition;
    WorkflowInstance instance(&definition);

    static ActiveState activeState;
    static InProgressState inProgressState;

    instance.setState(&activeState);

    WorkflowMemento* memento = instance.createMemento();

    REQUIRE(memento != nullptr);
    CHECK(memento->getState() == &activeState);

    instance.setState(&inProgressState);

    CHECK(instance.getState() == &inProgressState);

    instance.restore(memento);

    CHECK(instance.getState() == &activeState);

    delete memento;
}

TEST_CASE("Restoring one workflow instance does not affect another")
{
    WorkflowDefinition definition;

    WorkflowInstance first(&definition);
    WorkflowInstance second(&definition);

    static ActiveState activeState;
    static InProgressState inProgressState;

    first.setState(&activeState);
    second.setState(&activeState);

    WorkflowMemento* memento = first.createMemento();

    first.setState(&inProgressState);
    second.setState(&inProgressState);

    first.restore(memento);

    CHECK(first.getState() == &activeState);
    CHECK(second.getState() == &inProgressState);

    delete memento;
}

TEST_CASE("WorkflowInstance rejects an invalid memento")
{
    WorkflowDefinition definition;
    WorkflowInstance instance(&definition);

    CHECK_THROWS_AS(
        instance.restore(nullptr),
        std::invalid_argument
    );

    WorkflowMemento emptyMemento;

    CHECK_THROWS_AS(
        instance.restore(&emptyMemento),
        std::invalid_argument
    );
}