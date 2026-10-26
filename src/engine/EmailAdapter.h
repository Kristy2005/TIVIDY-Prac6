#ifndef EMAILADAPTER_H
#define EMAILADAPTER_H

#include "NotificationService.h"
#include "ExternalEmailService.h"
#include <string>


namespace tividy {
class EmailAdapter :public NotificationService {

public:
	ExternalEmailService* service;

	void sendNotification(Participant* recipient, std::string message);
};

#endif
}

