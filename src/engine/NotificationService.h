#ifndef NOTIFICATIONSERVICE_H
#define NOTIFICATIONSERVICE_H

class NotificationService {


public:
	virtual void sendNotification(Participant* recipient, string message) = 0;
};

#endif
