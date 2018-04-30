#ifndef CLIENTLISTENER_H_
#define CLIENTLISTENER_H_

class Client;
class ClientListener
{
public:
	ClientListener() {}
	virtual ~ClientListener() {}

	virtual void NotifyUpdateAvailable(Client * sender) {}
	virtual void NotifyAuthUserChanged(Client * sender) {}
	virtual void NotifyMessageOfTheDay(Client * sender) {}
	virtual void NotifyNewNotification(Client * sender, std::pair<String, ByteString> notification) {}
};


#endif /* CLIENTLISTENER_H_ */
