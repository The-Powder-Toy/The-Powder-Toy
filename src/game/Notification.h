/*
 * Notification.h
 *
 *  Created on: Jun 20, 2012
 *      Author: Simon
 */

#ifndef NOTIFICATION_H_
#define NOTIFICATION_H_

#include <string>

class Notification
{
public:
	Notification(std::string message) : Message(message) {}
	virtual ~Notification() {};
	std::string Message;

	virtual void Action() { }
};

#endif /* NOTIFICATION_H_ */
