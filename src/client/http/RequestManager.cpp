#ifndef NOHTTP
#include "RequestManager.h"

#include <iostream>

#include "Request.h"
#include "Config.h"

const int curl_multi_wait_timeout_ms = 100;
const long curl_max_host_connections = 6;

namespace http
{
	const long timeout = 15;
	ByteString proxy;
	ByteString user_agent;

	void RequestManager::Shutdown()
	{
		{
			std::lock_guard<std::mutex> g(rt_mutex);
			rt_shutting_down = true;
		}
		rt_cv.notify_one();

		if (initialized)
		{
			worker_thread.join();

			curl_multi_cleanup(multi);
			multi = NULL;
			curl_global_cleanup();
		}
	}

	void RequestManager::Initialise(ByteString Proxy)
	{
		curl_global_init(CURL_GLOBAL_DEFAULT);
		multi = curl_multi_init();
		if (multi)
		{
			curl_multi_setopt(multi, CURLMOPT_MAX_HOST_CONNECTIONS, curl_max_host_connections);
		}

		proxy = Proxy;

		user_agent = ByteString::Build("PowderToy/", SAVE_VERSION, ".", MINOR_VERSION, " (", IDENT_PLATFORM, "; ", IDENT_BUILD, "; M", MOD_ID, ") TPTPP/", SAVE_VERSION, ".", MINOR_VERSION, ".", BUILD_NUM, IDENT_RELTYPE, ".", SNAPSHOT_ID);

		worker_thread = std::thread([this]() { Worker(); });
		initialized = true;
	}

	void RequestManager::Worker()
	{
		bool shutting_down = false;
		while (!shutting_down)
		{
			{
				std::unique_lock<std::mutex> l(rt_mutex);
				if (!requests_added_to_multi)
				{
					while (!rt_shutting_down && requests_to_add.empty() && !requests_to_start && !requests_to_remove)
					{
						rt_cv.wait(l);
					}
				}
				shutting_down = rt_shutting_down;
				requests_to_remove = false;
				requests_to_start = false;
				for (Request *request : requests_to_add)
				{
					request->status = 0;
					requests.insert(request);
				}
				requests_to_add.clear();
			}

			if (multi && requests_added_to_multi)
			{
				int dontcare;
				struct CURLMsg *msg;

				curl_multi_wait(multi, nullptr, 0, curl_multi_wait_timeout_ms, &dontcare);
				curl_multi_perform(multi, &dontcare);
				while ((msg = curl_multi_info_read(multi, &dontcare)))
				{
					if (msg->msg == CURLMSG_DONE)
					{
						Request *request;
						curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &request);

						int finish_with = 600;

						switch (msg->data.result)
						{
						case CURLE_OK:
							long code;
							curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &code);
							finish_with = (int)code;
							break;
						
						case CURLE_UNSUPPORTED_PROTOCOL:  finish_with = 601; break;
						case CURLE_COULDNT_RESOLVE_HOST:  finish_with = 602; break;
						case CURLE_OPERATION_TIMEDOUT:    finish_with = 605; break;
						case CURLE_URL_MALFORMAT:         finish_with = 606; break;
						case CURLE_COULDNT_CONNECT:       finish_with = 607; break;
						case CURLE_COULDNT_RESOLVE_PROXY: finish_with = 608; break;
						case CURLE_TOO_MANY_REDIRECTS:    finish_with = 611; break;

						case CURLE_SSL_CONNECT_ERROR:        finish_with = 612; break;
						case CURLE_SSL_ENGINE_NOTFOUND:      finish_with = 613; break;
						case CURLE_SSL_ENGINE_SETFAILED:     finish_with = 614; break;
						case CURLE_SSL_CERTPROBLEM:          finish_with = 615; break;
						case CURLE_SSL_CIPHER:               finish_with = 616; break;
						case CURLE_SSL_ENGINE_INITFAILED:    finish_with = 617; break;
						case CURLE_SSL_CACERT_BADFILE:       finish_with = 618; break;
						case CURLE_SSL_CRL_BADFILE:          finish_with = 619; break;
						case CURLE_SSL_ISSUER_ERROR:         finish_with = 620; break;
						case CURLE_SSL_PINNEDPUBKEYNOTMATCH: finish_with = 621; break;
						case CURLE_SSL_INVALIDCERTSTATUS:    finish_with = 609; break;

						case CURLE_HTTP2:
						case CURLE_HTTP2_STREAM:
						
						case CURLE_FAILED_INIT:
						case CURLE_NOT_BUILT_IN:
						default:
							break;
						}

						if (finish_with >= 600)
						{
							std::cerr << request->error_buffer << std::endl;
						}

						request->status = finish_with;
					}
				};
			}

			std::set<Request *> requests_to_remove;
			for (Request *request : requests)
			{
				bool signal_done = false;

				{
					std::lock_guard<std::mutex> g(request->rm_mutex);
					if (shutting_down)
					{
						// In the weird case that a http::Request::Simple* call is
						// waiting on this Request, we should fail the request
						// instead of cancelling it ourselves.
						request->status = 610;
					}
					if (!request->rm_canceled && request->rm_started && !request->added_to_multi && !request->status)
					{
						if (multi && request->easy)
						{
							MultiAdd(request);
						}
						else
						{
							request->status = 604;
						}
					}
					if (!request->rm_canceled && request->rm_started && !request->rm_finished)
					{
						if (multi && request->easy)
						{
#ifdef REQUEST_USE_CURL_OFFSET_T
							curl_easy_getinfo(request->easy, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &request->rm_total);
							curl_easy_getinfo(request->easy, CURLINFO_SIZE_DOWNLOAD_T, &request->rm_done);
#else
							double total, done;
							curl_easy_getinfo(request->easy, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &total);
							curl_easy_getinfo(request->easy, CURLINFO_SIZE_DOWNLOAD, &done);
							request->rm_total = (curl_off_t)total;
							request->rm_done = (curl_off_t)done;
#endif
						}
						if (request->status)
						{
							request->rm_finished = true;
							MultiRemove(request);
							signal_done = true;
						}
					}
					if (request->rm_canceled)
					{
						requests_to_remove.insert(request);
					}
				}

				if (signal_done)
				{
					request->done_cv.notify_one();
				}
			}
			for (Request *request : requests_to_remove)
			{
				requests.erase(request);
				MultiRemove(request);
				delete request;
			}
		}
	}

	void RequestManager::MultiAdd(Request *request)
	{
		if (multi && request->easy && !request->added_to_multi)
		{
			curl_multi_add_handle(multi, request->easy);
			request->added_to_multi = true;
			++requests_added_to_multi;
		}
	}

	void RequestManager::MultiRemove(Request *request)
	{
		if (request->added_to_multi)
		{
			curl_multi_remove_handle(multi, request->easy);
			request->added_to_multi = false;
			--requests_added_to_multi;
		}
	}

	bool RequestManager::AddRequest(Request *request)
	{
		if (!initialized)
			return false;
		{
			std::lock_guard<std::mutex> g(rt_mutex);
			requests_to_add.insert(request);
		}
		rt_cv.notify_one();
		return true;
	}

	void RequestManager::StartRequest(Request *request)
	{
		{
			std::lock_guard<std::mutex> g(rt_mutex);
			requests_to_start = true;
		}
		rt_cv.notify_one();
	}

	void RequestManager::RemoveRequest(Request *request)
	{
		{
			std::lock_guard<std::mutex> g(rt_mutex);
			requests_to_remove = true;
		}
		rt_cv.notify_one();
	}
}
#endif
