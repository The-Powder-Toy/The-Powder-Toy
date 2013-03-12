#pragma once

class RequestListener
{
public:
	int ListenerRand;
	RequestListener() { ListenerRand = rand(); }
	virtual ~RequestListener() {}

	virtual void OnResponseReady(void * response) {}
};
