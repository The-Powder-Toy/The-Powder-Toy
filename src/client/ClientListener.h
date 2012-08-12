/*
 * ClientListener.h
 *
 *  Created on: Jun 19, 2012
 *      Author: Simon
 */

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
};


#endif /* CLIENTLISTENER_H_ */
