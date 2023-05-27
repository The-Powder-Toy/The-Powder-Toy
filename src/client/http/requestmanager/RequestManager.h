#pragma once
#include "common/ExplicitSingleton.h"
#include "common/String.h"
#include <atomic>
#include <thread>
#include <vector>
#include <memory>
#include <mutex>
#include <map>
#include <condition_variable>

namespace http
{
	class Request;

	struct RequestHandle
	{
	protected:
		struct CtorTag
		{
		};

	public:
		ByteString uri;
		ByteString verb;
		bool isPost = false;
		std::map<ByteString, ByteString> postData;
		std::vector<ByteString> headers;

		enum State
		{
			ready,
			running,
			done,
			dead,
		};
		State state = ready;
		std::mutex stateMx;
		std::condition_variable stateCv;
		std::atomic<int> bytesTotal = -1;
		std::atomic<int> bytesDone = 0;
		int statusCode = 0;
		ByteString responseData;
		std::vector<ByteString> responseHeaders;
		ByteString error;

		RequestHandle(CtorTag)
		{
		}

		RequestHandle(const RequestHandle &) = delete;
		RequestHandle &operator =(const RequestHandle &) = delete;

		static std::shared_ptr<RequestHandle> Create();
	};

	class RequestManager;
	struct RequestManagerDeleter
	{
		void operator ()(RequestManager *ptr) const;
	};
	using RequestManagerPtr = std::unique_ptr<RequestManager, RequestManagerDeleter>;
	class RequestManager : public ExplicitSingleton<RequestManager>
	{
		ByteString proxy;
		ByteString cafile;
		ByteString capath;
		ByteString userAgent;
		bool disableNetwork;

		std::thread worker;
		void InitWorker();
		void Worker();
		void ExitWorker();

		std::vector<std::shared_ptr<RequestHandle>> requestHandles;
		void RegisterRequestHandle(std::shared_ptr<RequestHandle> requestHandle);
		void UnregisterRequestHandle(std::shared_ptr<RequestHandle> requestHandle);
		void Tick();

		// State shared between Request threads and the worker thread.
		std::vector<std::shared_ptr<RequestHandle>> requestHandlesToRegister;
		std::vector<std::shared_ptr<RequestHandle>> requestHandlesToUnregister;
		bool running = true;
		std::mutex sharedStateMx;

	protected:
		RequestManager(ByteString newProxy, ByteString newCafile, ByteString newCapath, bool newDisableNetwork);

	public:
		~RequestManager();

		void RegisterRequest(Request &request);
		void UnregisterRequest(Request &request);

		bool DisableNetwork() const;

		static RequestManagerPtr Create(ByteString newProxy, ByteString newCafile, ByteString newCapath, bool newDisableNetwork);
	};

	constexpr int TickMs = 100;
}
