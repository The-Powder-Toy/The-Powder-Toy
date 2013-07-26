#include "RequestBroker.h"

class ImageRequest: public RequestBroker::Request
{
public:
	int Width, Height;
	std::string URL;
	int RequestTime;
	void * HTTPContext;
	ImageRequest(std::string url, int width, int height, ListenerHandle listener);
	virtual RequestBroker::ProcessResponse Process(RequestBroker & rb);
	virtual ~ImageRequest();
	virtual void Cleanup();
};
