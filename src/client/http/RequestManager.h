#ifndef REQUESTMANAGER_H
#define REQUESTMANAGER_H

#include "common/tpt-thread.h"
#include <ctime>
#include <set>
#include <curl/curl.h>
#include "common/Singleton.h"
#include "common/String.h"

namespace http
{
	class Request;
	class RequestManager : public Singleton<RequestManager>
	{
		pthread_t worker_thread;
		std::set<Request *> requests;

		std::set<Request *> requests_to_add;
		std::set<Request *> requests_to_remove;
		bool rt_shutting_down;
		pthread_mutex_t rt_mutex;
		pthread_cond_t rt_cv;

		CURLM *multi;

		void Start();
		void Worker();
		void AddRequest(Request *request);

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
