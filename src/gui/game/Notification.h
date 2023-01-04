#pragma once
#include "common/String.h"

class Notification
{
public:
	Notification(String message) : Message(message) {}
	virtual ~Notification() {};
	String Message;

	virtual void Action() { }
};
