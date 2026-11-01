#include "WorkflowHistory.h"
#include <stdexcept>
#include <ctime>
#include <sstream>

namespace tividy {

void WorkflowHistory::save(WorkflowMemento* memento) {
	if(memento == nullptr) {
		throw std::invalid_argument("WorkflowHistory::save: memento cannot be null");
	}
	this->mementos.push_back(memento);
}

WorkflowMemento* WorkflowHistory::getLast() {
	if(this->mementos.empty()) {
		throw std::logic_error("WorkflowHistory::getLast: no saved mementos");
	}
	return this->mementos.back();
	}

	void WorkflowHistory::recordEvent(
    const std::string& action,
    const std::string& workItem,
    const std::string& participant)
{
    std::time_t now = std::time(nullptr);

    char buffer[20];
    std::strftime(
        buffer,
        sizeof(buffer),
        "%Y-%m-%d %H:%M:%S",
        std::localtime(&now)
    );

    events.push_back(
        AuditEvent(
            buffer,
            action,
            workItem,
            participant
        )
    );
}

const std::vector<AuditEvent>&
WorkflowHistory::getEvents() const
{
    return events;
}

std::string WorkflowHistory::describeEvents() const
{
    std::ostringstream output;

    for (std::size_t i = 0; i < events.size(); ++i) {

        output << events[i].time
               << " - "
               << events[i].action;

        if (!events[i].workItem.empty()) {
            output << " | Work item: "
                   << events[i].workItem;
        }

        if (!events[i].participant.empty()) {
            output << " | Participant: "
                   << events[i].participant;
        }

        output << "\n";
    }

    return output.str();
}
}

} // namespace tividy
