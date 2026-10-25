#ifndef NOTIFICATIONSERVICE_H
#define NOTIFICATIONSERVICE_H

#include "Participant.h"
#include <string>

class NotificationService {


public:
	virtual void sendNotification(Participant* recipient, std::string message) = 0;
};

#endif
