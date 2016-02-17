#ifndef NOTIFICATION_H_
#define NOTIFICATION_H_

#include <string>
#include "Format.h"

class Notification
{
public:
	Notification(std::string message) : Message(format::StringToWString(message)) {}
	Notification(std::wstring message) : Message(message) {}
	virtual ~Notification() {};
	std::wstring Message;

	virtual void Action() { }
};

#endif /* NOTIFICATION_H_ */
