#ifndef EXTERNALEMAILSERVICE_H
#define EXTERNALEMAILSERVICE_H

#include <string>



namespace tividy {
class ExternalEmailService {


public:
	void sendEmail(std::string address, std::string body);
};

#endif
}

