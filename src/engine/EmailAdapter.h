#ifndef EMAILADAPTER_H
#define EMAILADAPTER_H

class EmailAdapter : NotificationService {

public:
	ExternalEmailService* service;

	void sendNotification(Participant* recipient, string message);
};

#endif
