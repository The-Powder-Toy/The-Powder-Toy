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
			Worker();
		});
	}

	RequestManager::~RequestManager()
	{
		{
			std::lock_guard lk(sharedStateMx);
			running = false;
		}
		worker.join();
	}

	void RequestManager::Worker()
	{
		InitWorker();
		while (true)
		{
			{
				std::lock_guard lk(sharedStateMx);
				for (auto &requestHandle : requestHandles)
				{
					if (requestHandle->statusCode)
					{
						requestHandlesToUnregister.push_back(requestHandle);
					}
				}
				for (auto &requestHandle : requestHandlesToRegister)
				{
					requestHandles.push_back(requestHandle);
					RegisterRequestHandle(requestHandle);
				}
				requestHandlesToRegister.clear();
				for (auto &requestHandle : requestHandlesToUnregister)
				{
					auto eraseFrom = std::remove(requestHandles.begin(), requestHandles.end(), requestHandle);
					if (eraseFrom != requestHandles.end())
					{
						assert(eraseFrom + 1 == requestHandles.end());
						UnregisterRequestHandle(requestHandle);
						requestHandles.erase(eraseFrom, requestHandles.end());
						if (requestHandle->error.size())
						{
							std::cerr << requestHandle->error << std::endl;
						}
						{
							std::lock_guard lk(requestHandle->stateMx);
							requestHandle->state = RequestHandle::done;
						}
						requestHandle->stateCv.notify_one();
					}
				}
				requestHandlesToUnregister.clear();
				if (!running)
				{
					break;
				}
			}
			Tick();
		}
		assert(!requestHandles.size());
		ExitWorker();
	}

	bool RequestManager::DisableNetwork() const
	{
		return disableNetwork;
	}

	void RequestManager::RegisterRequest(Request &request)
	{
		std::lock_guard lk(sharedStateMx);
		requestHandlesToRegister.push_back(request.handle);
	}

	void RequestManager::UnregisterRequest(Request &request)
	{
		std::lock_guard lk(sharedStateMx);
		requestHandlesToUnregister.push_back(request.handle);
	}
}
