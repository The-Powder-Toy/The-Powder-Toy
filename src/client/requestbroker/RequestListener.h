#pragma once

class RequestListener
{
public:
	static unsigned int nextListenerID;
	int ListenerID;
	RequestListener() { ListenerID = nextListenerID++; }
	virtual ~RequestListener() {}

	virtual void OnResponseReady(void * response, int identifier) {}
	virtual void OnResponseFailed(int identifier) {}
};
