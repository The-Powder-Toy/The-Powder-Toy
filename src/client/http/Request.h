#ifndef REQUEST_H
#define REQUEST_H

#include <map>
#include "common/tpt-minmax.h" // for MSVC, ensures windows.h doesn't cause compile errors by defining min/max
#include "common/tpt-thread.h"
#include <curl/curl.h>
#include "common/String.h"
#undef GetUserName // pthreads defines this, breaks stuff

#if defined(CURL_AT_LEAST_VERSION) && CURL_AT_LEAST_VERSION(7, 55, 0)
# define REQUEST_USE_CURL_OFFSET_T
#endif

#if defined(CURL_AT_LEAST_VERSION) && CURL_AT_LEAST_VERSION(7, 56, 0)
# define REQUEST_USE_CURL_MIMEPOST
#endif

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

#ifdef REQUEST_USE_CURL_MIMEPOST
		curl_mime *post_fields;
#else
		curl_httppost *post_fields_first, *post_fields_last;
		std::map<ByteString, ByteString> post_fields_map;
#endif

		pthread_cond_t done_cv;

		static size_t WriteDataHandler(char * ptr, size_t size, size_t count, void * userdata);

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

	String StatusText(int code);

	extern const long timeout;
	extern ByteString proxy;
	extern ByteString user_agent;
}

#endif // REQUEST_H
