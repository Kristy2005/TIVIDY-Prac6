/**
 * @file TestFixtures.h
 * @brief A small generic definition ("request approval") used by engine tests. Not an industry scenario.
 *
 *  intake(clerk) -> review(reviewer) -+-[approved]-> fulfil(clerk) ------+-> archive(clerk)
 *                                     +-[rejected]-> reject_notice(clerk)-+
 */
#ifndef TIVIDY_TEST_FIXTURES_H
#define TIVIDY_TEST_FIXTURES_H

#include <memory>
#include "engine/WorkflowBuilder.h"
#include "engine/WorkflowInstance.h"

namespace fixtures {

inline std::shared_ptr<tividy::WorkflowDefinition> approvalDefinition() {
    using namespace tividy;
    ConcreteWorkflowBuilder b("Request approval");
    b.addRole("clerk");
    b.addRole("reviewer");
    b.addRole("manager");
    b.addStage("main", "Main");
    b.addWorkItem("main", WorkItemSpec("intake", "Capture request", "clerk"));
    b.addWorkItem("main", WorkItemSpec("review", "Review request", "reviewer", Priority::High).after("intake"));
    b.addWorkItem("main", WorkItemSpec("fulfil", "Fulfil request", "clerk"));
    b.addWorkItem("main", WorkItemSpec("reject_notice", "Send rejection notice", "clerk"));
    b.addWorkItem("main", WorkItemSpec("archive", "Archive case", "clerk").after("fulfil").after("reject_notice"));
    b.addRoutingRule("review", "approved", "fulfil");
    b.addRoutingRule("review", "rejected", "reject_notice");
    b.addEscalationHandler(new ApprovalHandler("reviewer", 1));
    b.addEscalationHandler(new FinalDecisionHandler("manager"));
    return b.build();
}

/** @brief Instance with alice (clerk), bob (reviewer) and carol (manager) registered. */
inline std::unique_ptr<tividy::WorkflowInstance> newInstance(const std::string& id,
                                                             std::shared_ptr<tividy::WorkflowDefinition> def) {
    std::unique_ptr<tividy::WorkflowInstance> w(new tividy::WorkflowInstance(id, def));
    w->addParticipant("alice", "clerk");
    w->addParticipant("bob", "reviewer");
    w->addParticipant("carol", "manager");
    return w;
}

} // namespace fixtures

#endif
