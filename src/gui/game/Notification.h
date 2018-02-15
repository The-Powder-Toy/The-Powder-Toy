#ifndef NOTIFICATION_H_
#define NOTIFICATION_H_

#include <string>
#include <utility>

class Notification
{
public:
	Notification(std::string message) : Message(std::move(message)) {}
	virtual ~Notification() = default;;
	std::string Message;

	virtual void Action() { }
};

#endif /* NOTIFICATION_H_ */
