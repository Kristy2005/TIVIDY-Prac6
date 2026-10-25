#include "engine/Handler.h"

namespace tividy {

Handler::Handler(const std::string& role, int authority) : role_(role), authority_(authority) {}
Handler::~Handler() {}

Handler* Handler::append(Handler* next) {
    if (!next) return 0;
    Handler* last = this;
    while (last->next_) last = last->next_.get();
    last->next_.reset(next);
    return next;
}

bool Handler::canHandle(const EscalationRequest& request) const { return authority_ >= request.requiredLevel; }

bool Handler::handle(EscalationRequest& request) const {
    if (canHandle(request)) {
        for (size_t i = 0; i < request.participants.size(); ++i) {
            Participant* p = request.participants[i];
            if (p->isAvailable() && p->role().name() == role_) {
                request.trail.push_back(kind() + "(" + role_ + ") accepted");
                request.handledBy = p;
                return true;
            }
        }
        request.trail.push_back(kind() + "(" + role_ + ") has nobody available");
    } else {
        request.trail.push_back(kind() + "(" + role_ + ") passed it on");
    }
    return next_ ? next_->handle(request) : false;
}

} // namespace tividy
