#ifndef NOTIFICATION_H_
#define NOTIFICATION_H_

#include "common/String.h"

class Notification
{
public:
	Notification(String message) : Message(message) {}
	virtual ~Notification() {};
	String Message;

	virtual void Action() { }
};

#endif /* NOTIFICATION_H_ */
