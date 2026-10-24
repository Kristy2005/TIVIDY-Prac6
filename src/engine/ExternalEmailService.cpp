#include "engine/ExternalEmailService.h"

namespace tividy {

int ExternalEmailService::transmit(const std::string& address, const std::string& raw) {
    if (!online_) return 503;
    Mail m;
    m.address = address;
    m.raw = raw;
    outbox_.push_back(m);
    return 0;
}

} // namespace tividy
