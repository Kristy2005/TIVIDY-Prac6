#include "doctest.h"

#include "engine/WorkItem.h"
#include "engine/WorkflowStage.h"
#include "engine/WorkflowDefinition.h"
#include "engine/WorkflowInstance.h"

using namespace tividy;

TEST_CASE("Prototype clones a WorkItem independently")
{
    WorkItem original("Review complaint", "Administrator");

    WorkflowComponent* clonedComponent = original.clone();

    REQUIRE(clonedComponent != nullptr);
    CHECK(clonedComponent != &original);

    WorkItem* clone = dynamic_cast<WorkItem*>(clonedComponent);

    REQUIRE(clone != nullptr);

    CHECK(clone->getName() == original.getName());
    CHECK(clone->getRequiredRole() == original.getRequiredRole());
    CHECK_FALSE(clone->isComplete());

    clone->complete();

    CHECK(clone->isComplete());
    CHECK_FALSE(original.isComplete());

    delete clonedComponent;
}

TEST_CASE("Prototype deep clones a WorkflowStage")
{
    WorkItem first("First task");
    WorkItem second("Second task");

    WorkflowStage original;
    original.add(&first);
    original.add(&second);

    WorkflowComponent* clonedComponent = original.clone();

    REQUIRE(clonedComponent != nullptr);
    CHECK(clonedComponent != &original);

    WorkflowStage* clonedStage =
        dynamic_cast<WorkflowStage*>(clonedComponent);

    REQUIRE(clonedStage != nullptr);

    std::vector<WorkItem*> originalItems;
    std::vector<WorkItem*> clonedItems;

    original.collectWorkItems(originalItems);
    clonedStage->collectWorkItems(clonedItems);

    REQUIRE(originalItems.size() == 2);
    REQUIRE(clonedItems.size() == 2);

    CHECK(originalItems[0] != clonedItems[0]);
    CHECK(originalItems[1] != clonedItems[1]);

    CHECK(originalItems[0]->getName() == clonedItems[0]->getName());
    CHECK(originalItems[1]->getName() == clonedItems[1]->getName());

    clonedItems[0]->complete();

    CHECK(clonedItems[0]->isComplete());
    CHECK_FALSE(originalItems[0]->isComplete());

    delete clonedComponent;
}

TEST_CASE("WorkflowDefinition creates independent work trees")
{
    WorkItem item("Investigate complaint");

    WorkflowStage stage;
    stage.add(&item);

    WorkflowDefinition definition;
    definition.setRoot(&stage);

    WorkflowComponent* firstTree =
        definition.createWorkTree();

    WorkflowComponent* secondTree =
        definition.createWorkTree();

    REQUIRE(firstTree != nullptr);
    REQUIRE(secondTree != nullptr);

    CHECK(firstTree != secondTree);

    std::vector<WorkItem*> firstItems;
    std::vector<WorkItem*> secondItems;

    firstTree->collectWorkItems(firstItems);
    secondTree->collectWorkItems(secondItems);

    REQUIRE(firstItems.size() == 1);
    REQUIRE(secondItems.size() == 1);

    CHECK(firstItems[0] != secondItems[0]);

    firstItems[0]->complete();

    CHECK(firstItems[0]->isComplete());
    CHECK_FALSE(secondItems[0]->isComplete());
    CHECK_FALSE(item.isComplete());

    delete firstTree;
    delete secondTree;
}

TEST_CASE("WorkflowInstances created from one definition remain independent")
{
    WorkItem item("Review application");

    WorkflowStage stage;
    stage.add(&item);

    WorkflowDefinition definition;
    definition.setRoot(&stage);

    WorkflowInstance first(&definition);
    WorkflowInstance second(&definition);

    CHECK_FALSE(first.isWorkItemComplete(&item));
    CHECK_FALSE(second.isWorkItemComplete(&item));

    first.completeWorkItem(&item);

    CHECK(first.isWorkItemComplete(&item));
    CHECK_FALSE(second.isWorkItemComplete(&item));
}