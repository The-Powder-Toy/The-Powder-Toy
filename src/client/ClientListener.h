#pragma once
#include "common/String.h"
#include "client/ServerNotification.h"

class Client;
class ClientListener
{
public:
	ClientListener() {}
	virtual ~ClientListener() {}

	virtual void NotifyUpdateAvailable(Client * sender) {}
	virtual void NotifyAuthUserChanged(Client * sender) {}
	virtual void NotifyMessageOfTheDay(Client * sender) {}
	virtual void NotifyNewNotification(Client * sender, ServerNotification notification) {}
};

