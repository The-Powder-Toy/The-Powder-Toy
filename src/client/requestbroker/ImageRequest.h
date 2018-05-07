#include "RequestBroker.h"

class ImageRequest: public RequestBroker::Request
{
public:
	int Width, Height;
	ByteString URL;
	int RequestTime;
	void * HTTPContext;
	bool started = false;
	ImageRequest(ByteString url, int width, int height, ListenerHandle listener, int identifier = 0);
	virtual RequestBroker::ProcessResponse Process(RequestBroker & rb);
	virtual ~ImageRequest();
	virtual void Cleanup();
};
