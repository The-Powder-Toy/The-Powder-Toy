#include "RequestManager.h"
#include "client/http/Request.h"
#include "Config.h"
#include <iostream>

namespace http
{
	RequestManager::RequestManager(ByteString newProxy, ByteString newCafile, ByteString newCapath, bool newDisableNetwork) :
		proxy(newProxy),
		cafile(newCafile),
		capath(newCapath),
		disableNetwork(newDisableNetwork)
	{
		userAgent = ByteString::Build(
			"PowderToy/", SAVE_VERSION, ".", MINOR_VERSION,
			" (", IDENT_PLATFORM,
			"; NO", // Unused, used to be SSE level.
			"; M", MOD_ID,
			"; ", IDENT,
			") TPTPP/", SAVE_VERSION, ".", MINOR_VERSION, ".", BUILD_NUM, IDENT_RELTYPE, ".", SNAPSHOT_ID
		);
		worker = std::thread([this]() {
			Run();
		});
	}

	RequestManager::~RequestManager()
	{
		{
			std::lock_guard lk(sharedStateMx);
			running = false;
		}
		sharedStateCv.notify_all();
		worker.join();
	}

	bool RequestManager::ProcessEvents(bool shouldWait)
	{
		std::unique_lock lk(sharedStateMx);
		if (shouldWait)
		{
			sharedStateCv.wait(lk);
		}

		for (auto &requestHandle : requestHandlesToRegister)
		{
			requestHandles.push_back(requestHandle);
			RegisterRequestHandle(requestHandle);
		}
		requestHandlesToRegister.clear();
		for (auto &requestHandle : requestHandlesToUnregister)
		{
			RequestDone(requestHandle);
		}
		requestHandlesToUnregister.clear();

		return running;
	}

	void RequestManager::RequestDone(std::shared_ptr<RequestHandle> &requestHandle)
	{
		auto toRemove = std::find(requestHandles.begin(), requestHandles.end(), requestHandle);
		if (toRemove != requestHandles.end()) RemoveRequest(toRemove);
	}

	void RequestManager::RequestDone(RequestHandle *handle)
	{
		auto toRemove = std::find_if(requestHandles.begin(), requestHandles.end(), [handle] (const std::shared_ptr<RequestHandle> &sptr) {
			return handle == sptr.get();
		});
		RemoveRequest(toRemove);
	}

	void RequestManager::RemoveRequest(std::vector<std::shared_ptr<RequestHandle>>::iterator toRemove) {
		assert(toRemove != requestHandles.end());
		// swap removed request to end before removing
		auto swapTo = requestHandles.end() - 1;
		std::swap(*toRemove, *swapTo);
		auto requestHandle = *swapTo;
		UnregisterRequestHandle(requestHandle);
		requestHandles.erase(swapTo, requestHandles.end());
		if (!requestHandle->error.empty())
		{
			std::cerr << requestHandle->error << std::endl;
		}
		{
			std::lock_guard handle_lock(requestHandle->stateMx);
			requestHandle->state = RequestHandle::done;
		}
		requestHandle->stateCv.notify_one();
	}

	bool RequestManager::DisableNetwork() const
	{
		return disableNetwork;
	}

	void RequestManager::RegisterRequest(Request &request)
	{
		{
			std::lock_guard lk(sharedStateMx);
			requestHandlesToRegister.push_back(request.handle);
		}
		sharedStateCv.notify_one();
	}

	void RequestManager::UnregisterRequest(Request &request)
	{
		{
			std::lock_guard lk(sharedStateMx);
			requestHandlesToUnregister.push_back(request.handle);
		}
		sharedStateCv.notify_one();
	}
}
