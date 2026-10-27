#include "NotificationObserver.h"
#include <stdexcept>
namespace tividy {
void NotificationObserver::update(WorkflowInstance* instance) {
    if (!instance) throw std::invalid_argument("NotificationObserver: null instance");
}
}
