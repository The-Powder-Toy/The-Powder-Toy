#include "RequestManager.h"
#include "client/http/Request.h"

namespace http
{
	std::shared_ptr<RequestHandle> RequestHandle::Create()
	{
		return std::make_shared<RequestHandle>(CtorTag{});
	}

	void RequestManager::Run()
	{
		while (true)
		{
			bool shouldWait = requestHandles.empty();
			if (!ProcessEvents(shouldWait)) break;
		}
	}

	void RequestManager::InitWorker()
	{
	}

	void RequestManager::ExitWorker()
	{
	}

	void RequestManager::RegisterRequestHandle(std::shared_ptr<RequestHandle> requestHandle)
	{
		requestHandle->statusCode = 604;
		requestHandle->error = "network support not compiled in";
		RequestDone(requestHandle);
	}

	void RequestManager::UnregisterRequestHandle(std::shared_ptr<RequestHandle> requestHandle)
	{
	}

	void RequestManager::Tick()
	{
	}

	RequestManagerPtr RequestManager::Create(ByteString newProxy, ByteString newCafile, ByteString newCapath, bool newDisableNetwork)
	{
		return RequestManagerPtr(new RequestManager(newProxy, newCafile, newCapath, newDisableNetwork));
	}

	void RequestManagerDeleter::operator ()(RequestManager *ptr) const
	{
		delete ptr;
	}
}
