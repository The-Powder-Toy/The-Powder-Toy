#pragma once
#include "common/String.h"
#include "PostData.h"
#include <map>
#include <utility>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <optional>

namespace http
{
	struct RequestHandle;

	// Thrown by Finish and ParseResponse
	struct RequestError : public std::runtime_error
	{
		using runtime_error::runtime_error;
	};

	class Request
	{
		std::shared_ptr<RequestHandle> handle;

	public:
		Request(ByteString newUri);
		Request(const Request &) = delete;
		Request &operator =(const Request &) = delete;
		~Request();

		void FailEarly(ByteString error);

		void Verb(ByteString newVerb);
		void AddHeader(Header header);

		void AddPostData(PostData data);
		void AuthHeaders(ByteString ID, ByteString session);

		void Start();
		bool CheckDone() const;

		std::pair<int64_t, int64_t> CheckProgress() const; // total, done
		const std::vector<Header> &ResponseHeaders() const;
		void Wait();

		int StatusCode() const; // status
		std::pair<int, ByteString> Finish(); // status, data

		enum ResponseType
		{
			responseOk,
			responseJson,
			responseData,
		};
		static void ParseResponse(const ByteString &result, int status, ResponseType responseType);

		friend class RequestManager;
	};

	const char *StatusText(int code);
}
