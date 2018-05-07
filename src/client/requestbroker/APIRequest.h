#include <map>
#include "RequestBroker.h"

class APIResultParser;
class APIRequest: public RequestBroker::Request
{
public:
	bool Post;
	APIResultParser * Parser;
	ByteString URL;
	std::map<ByteString, ByteString> PostData;
	void * HTTPContext;
	APIRequest(ByteString url, APIResultParser * parser, ListenerHandle listener = ListenerHandle(0, (RequestListener*)0), int identifier = 0);
	APIRequest(ByteString url, std::map<ByteString, ByteString>, APIResultParser * parser, ListenerHandle listener = ListenerHandle(0, (RequestListener*)0), int identifier = 0);
	virtual RequestBroker::ProcessResponse Process(RequestBroker & rb);
	virtual ~APIRequest();
	virtual void Cleanup();
};

