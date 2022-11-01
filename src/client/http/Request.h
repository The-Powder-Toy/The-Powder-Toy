#ifndef REQUEST_H
#define REQUEST_H
#include "Config.h"

#ifndef NOHTTP
# include <curl/curl.h>
# include "common/tpt-minmax.h" // for MSVC, ensures windows.h doesn't cause compile errors by defining min/max
# include <mutex>
# include <condition_variable>
# if defined(CURL_AT_LEAST_VERSION) && CURL_AT_LEAST_VERSION(7, 55, 0)
#  define REQUEST_USE_CURL_OFFSET_T
# endif
# if defined(CURL_AT_LEAST_VERSION) && CURL_AT_LEAST_VERSION(7, 56, 0)
#  define REQUEST_USE_CURL_MIMEPOST
# endif
# if defined(CURL_AT_LEAST_VERSION) && CURL_AT_LEAST_VERSION(7, 61, 0)
#  define REQUEST_USE_CURL_TLSV13CL
# endif
#endif

#include <map>
#include "common/String.h"

namespace http
{
	class RequestManager;
	class Request
	{
#ifndef NOHTTP
		ByteString uri;
		std::vector<ByteString> response_headers;
		ByteString response_body;

		CURL *easy;
		char error_buffer[CURL_ERROR_SIZE];

		volatile curl_off_t rm_total;
		volatile curl_off_t rm_done;
		volatile bool rm_finished;
		volatile bool rm_canceled;
		volatile bool rm_started;
		std::mutex rm_mutex;

		bool added_to_multi;
		int status;

		ByteString verb;
		struct curl_slist *headers;

		bool isPost = false;
#ifdef REQUEST_USE_CURL_MIMEPOST
		curl_mime *post_fields;
#else
		curl_httppost *post_fields_first, *post_fields_last;
		std::map<ByteString, ByteString> post_fields_map;
#endif

		std::condition_variable done_cv;

		static size_t HeaderDataHandler(char *ptr, size_t size, size_t count, void *userdata);
		static size_t WriteDataHandler(char *ptr, size_t size, size_t count, void *userdata);
#endif

	public:
		Request(ByteString uri);
		virtual ~Request();

		void Verb(ByteString newVerb);
		void AddHeader(ByteString header);
		void AddPostData(std::map<ByteString, ByteString> data);
		void AuthHeaders(ByteString ID, ByteString session);

		void Start();
		ByteString Finish(int *status, std::vector<ByteString> *headers = nullptr);
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
}

#endif // REQUEST_H
