#ifndef REQUESTMANAGER_H
#define REQUESTMANAGER_H

#include "common/tpt-minmax.h" // for MSVC, ensures windows.h doesn't cause compile errors by defining min/max
#include "common/tpt-thread.h"
#include <ctime>
#include <set>
#include <curl/curl.h>
#include "common/Singleton.h"
#include "common/String.h"
#undef GetUserName // pthreads (included by curl) defines this, breaks stuff

namespace http
{
	class Request;
	class RequestManager : public Singleton<RequestManager>
	{
		pthread_t worker_thread;
		std::set<Request *> requests;
		int requests_added_to_multi;

		std::set<Request *> requests_to_add;
		bool requests_to_start;
		bool requests_to_remove;
		bool rt_shutting_down;
		pthread_mutex_t rt_mutex;
		pthread_cond_t rt_cv;

		CURLM *multi;

		void Start();
		void Worker();
		void MultiAdd(Request *request);
		void MultiRemove(Request *request);
		void AddRequest(Request *request);
		void StartRequest(Request *request);
		void RemoveRequest(Request *request);

		static TH_ENTRY_POINT void *RequestManagerHelper(void *obj);

	public:
		RequestManager();
		~RequestManager();

		void Initialise(ByteString proxy);
		void Shutdown();

		friend class Request;
	};

	extern const long timeout;
	extern ByteString proxy;
	extern ByteString user_agent;
}

#endif // REQUESTMANAGER_H
