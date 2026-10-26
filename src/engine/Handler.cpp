#include "Handler.h"

namespace tividy {

void Handler::setSuccessor(Handler* handler) {
	this->successor = handler;
}

void Handler::handleRequest(WorkflowInstance* instance) {
	// TODO - implement Handler::handleRequest
	throw "Not yet implemented";
}
}
