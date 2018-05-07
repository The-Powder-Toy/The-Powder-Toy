#include <map>
#include "RequestBroker.h"

class WebRequest: public RequestBroker::Request
{
public:
	bool Post;
	ByteString URL;
	std::map<ByteString, ByteString> PostData;
	void * HTTPContext;
	WebRequest(ByteString url, ListenerHandle listener = ListenerHandle(0, (RequestListener*)0), int identifier = 0);
	WebRequest(ByteString url, std::map<ByteString, ByteString>, ListenerHandle listener = ListenerHandle(0, (RequestListener*)0), int identifier = 0);
	virtual RequestBroker::ProcessResponse Process(RequestBroker & rb);
	virtual ~WebRequest();
	virtual void Cleanup();
};

