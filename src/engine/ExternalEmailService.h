#ifndef EXTERNALEMAILSERVICE_H
#define EXTERNALEMAILSERVICE_H

#include <string>


class ExternalEmailService {


public:
	void sendEmail(std::string address, std::string body);
};

#endif
