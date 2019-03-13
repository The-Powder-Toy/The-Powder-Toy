#ifndef REQUEST_H
#define REQUEST_H

#include <map>
#include <curl/curl.h>
#include "common/String.h"

namespace http
{
	class RequestManager;
	class Request
	{
		ByteString uri;
		ByteString response_body;

		CURL *easy;

		volatile curl_off_t rm_total;
		volatile curl_off_t rm_done;
		volatile bool rm_finished;
		volatile bool rm_canceled;
		volatile bool rm_started;
		pthread_mutex_t rm_mutex;

		bool added_to_multi;
		int status;

		struct curl_slist *headers;
		curl_mime *post_fields;

		pthread_cond_t done_cv;

	public:
		Request(ByteString uri);
		virtual ~Request();

		void AddHeader(ByteString name, ByteString value);
		void AddPostData(std::map<ByteString, ByteString> data);
		void AuthHeaders(ByteString ID, ByteString session);

		void Start();
		ByteString Finish(int *status);
		void Cancel();

		void CheckProgress(int *total, int *done);
		bool CheckDone();
		bool CheckCanceled();
		bool CheckStarted();

		friend class RequestManager;

		static ByteString Simple(ByteString uri, int *status, std::map<ByteString, ByteString> post_data = std::map<ByteString, ByteString>{});
		static ByteString SimpleAuth(ByteString uri, int *status, ByteString ID, ByteString session, std::map<ByteString, ByteString> post_data = std::map<ByteString, ByteString>{});
	};

	const char *StatusText(int code);

	extern const long timeout;
	extern ByteString proxy;
	extern ByteString user_agent;
}

#endif // REQUEST_H
