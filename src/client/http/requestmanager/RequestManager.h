#pragma once
#include "common/ExplicitSingleton.h"
#include "common/String.h"
#include "client/http/PostData.h"
#include <atomic>
#include <cstdint>
#include <thread>
#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <utility>

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
		std::optional<ByteString> verb;
		bool isPost = false;
		PostData postData;
		std::vector<Header> headers;

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
		std::atomic<int64_t> bytesTotal = -1;
		std::atomic<int64_t> bytesDone = 0;
		int statusCode = 0;
		ByteString responseData;
		std::vector<Header> responseHeaders;
		std::optional<ByteString> error;
		std::optional<ByteString> failEarly;

		RequestHandle(CtorTag)
		{
		}

		RequestHandle(const RequestHandle &) = delete;
		RequestHandle &operator =(const RequestHandle &) = delete;

		void MarkDone();

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
	public:
		struct Config
		{
			std::optional<ByteString> proxy;
			std::optional<ByteString> cafile;
			std::optional<ByteString> capath;
			bool disableNetwork = false;
		};

	protected:
		Config config;
		ByteString userAgent;

		RequestManager(Config newConfig);

		void RegisterRequestImpl(Request &request);
		void UnregisterRequestImpl(Request &request);

	public:
		void RegisterRequest(Request &request);
		void UnregisterRequest(Request &request);

		bool DisableNetwork() const
		{
			return config.disableNetwork;
		}

		const std::optional<ByteString> &Cafile() const
		{
			return config.cafile;
		}

		const std::optional<ByteString> &Capath() const
		{
			return config.capath;
		}

		const std::optional<ByteString> &Proxy() const
		{
			return config.proxy;
		}

		static RequestManagerPtr Create(Config newConfig);
	};
}
