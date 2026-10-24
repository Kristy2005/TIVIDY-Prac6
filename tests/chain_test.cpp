#include "doctest.h"
#include "engine/Handler.h"

using namespace tividy;

namespace {
struct Staff {
    Participant clerk, lead, boss;
    Staff() : clerk("cleo", Role("clerk")), lead("lena", Role("lead")), boss("bruno", Role("boss")) {}
    void addTo(EscalationRequest& r) {
        r.participants.push_back(&clerk);
        r.participants.push_back(&lead);
        r.participants.push_back(&boss);
    }
};
Handler* buildChain() {
    Handler* head = new WorkHandler("clerk", 1);
    head->append(new ApprovalHandler("lead", 2))->append(new FinalDecisionHandler("boss"));
    return head;
}
}

TEST_CASE("The first handler with enough authority accepts") {
    Staff s;
    std::unique_ptr<Handler> chain(buildChain());
    EscalationRequest r;
    s.addTo(r);
    r.requiredLevel = 1;
    CHECK(chain->handle(r));
    CHECK(r.handledBy == &s.clerk);
}

TEST_CASE("A higher required level is passed along the chain") {
    Staff s;
    std::unique_ptr<Handler> chain(buildChain());
    EscalationRequest r;
    s.addTo(r);
    r.requiredLevel = 2;
    CHECK(chain->handle(r));
    CHECK(r.handledBy == &s.lead);
    REQUIRE(r.trail.size() == 2);
    CHECK(r.trail[0] == "Work(clerk) passed it on");
    CHECK(r.trail[1] == "Approval(lead) accepted");
}

TEST_CASE("FinalDecisionHandler accepts any level") {
    Staff s;
    std::unique_ptr<Handler> chain(buildChain());
    EscalationRequest r;
    s.addTo(r);
    r.requiredLevel = 99;
    CHECK(chain->handle(r));
    CHECK(r.handledBy == &s.boss);
}

TEST_CASE("An absent handler is skipped, and an empty result is reported") {
    Staff s;
    std::unique_ptr<Handler> chain(buildChain());
    s.lead.setAvailable(false);
    EscalationRequest r;
    s.addTo(r);
    r.requiredLevel = 2;
    CHECK(chain->handle(r));
    CHECK(r.handledBy == &s.boss);

    s.boss.setAvailable(false);
    EscalationRequest r2;
    s.addTo(r2);
    r2.requiredLevel = 2;
    CHECK_FALSE(chain->handle(r2));
    CHECK(r2.handledBy == static_cast<Participant*>(0));
}
