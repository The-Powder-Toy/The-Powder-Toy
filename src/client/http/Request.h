#pragma once
#include "common/String.h"
#include "PostData.h"
#include <map>
#include <utility>
#include <vector>
#include <mutex>
#include <condition_variable>

namespace http
{
	struct RequestHandle;

	class Request
	{
		std::shared_ptr<RequestHandle> handle;

	public:
		Request(ByteString newUri);
		Request(const Request &) = delete;
		Request &operator =(const Request &) = delete;
		~Request();

		void Verb(ByteString newVerb);
		void AddHeader(ByteString header);

		void AddPostData(PostData data);
		void AuthHeaders(ByteString ID, ByteString session);

		void Start();
		bool CheckDone() const;

		std::pair<int, int> CheckProgress() const; // total, done
		const std::vector<ByteString> &ResponseHeaders() const;
		std::pair<int, ByteString> Finish(); // status, data

		static std::pair<int, ByteString> Simple(ByteString uri, FormData postData = {});
		static std::pair<int, ByteString> SimpleAuth(ByteString uri, ByteString ID, ByteString session, FormData postData = {});

		friend class RequestManager;
	};

	String StatusText(int code);
}
