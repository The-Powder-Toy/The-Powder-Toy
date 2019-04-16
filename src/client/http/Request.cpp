#include "Request.h"
#include "RequestManager.h"
#include "Platform.h"

namespace http
{
	Request::Request(ByteString uri_):
		uri(uri_),
		rm_total(0),
		rm_done(0),
		rm_finished(false),
		rm_canceled(false),
		rm_started(false),
		added_to_multi(false),
		status(0),
		headers(NULL),
#ifdef REQUEST_USE_CURL_MIMEPOST
		post_fields(NULL)
#else
		post_fields_first(NULL),
		post_fields_last(NULL)
#endif
	{
		easy = curl_easy_init();
		RequestManager::Ref().AddRequest(this);
	}

	Request::~Request()
	{
		curl_easy_cleanup(easy);
#ifdef REQUEST_USE_CURL_MIMEPOST
		curl_mime_free(post_fields);
#else
		curl_formfree(post_fields_first);
#endif
		curl_slist_free_all(headers);
	}

	void Request::AddHeader(ByteString name, ByteString value)
	{
		headers = curl_slist_append(headers, (name + ": " + value).c_str());
	}

	// add post data to a request
	void Request::AddPostData(std::map<ByteString, ByteString> data)
	{
		if (!data.size())
		{
			return;
		}

		if (easy)
		{
#ifdef REQUEST_USE_CURL_MIMEPOST
			if (!post_fields)
			{
				post_fields = curl_mime_init(easy);
			}

			for (auto &field : data)
			{
				curl_mimepart *part = curl_mime_addpart(post_fields);
				curl_mime_data(part, &field.second[0], field.second.size());
				if (auto split = field.first.SplitBy(':'))
				{
					curl_mime_name(part, split.Before().c_str());
					curl_mime_filename(part, split.After().c_str());
				}
				else
				{
					curl_mime_name(part, field.first.c_str());
				}
			}
#else
			post_fields_map.insert(data.begin(), data.end());
#endif
		}
	}

	// add userID and sessionID headers to the request
	void Request::AuthHeaders(ByteString ID, ByteString session)
	{
		if (ID.size())
		{
			if (session.size())
			{
				AddHeader("X-Auth-User-Id", ID);
				AddHeader("X-Auth-Session-Key", session);
			}
			else
			{
				AddHeader("X-Auth-User", ID);
			}
		}
	}

	size_t Request::WriteDataHandler(char *ptr, size_t size, size_t count, void *userdata)
	{
		Request *req = (Request *)userdata;
		auto actual_size = size * count;
		req->response_body.append(ptr, actual_size);
		return actual_size;
	}

	// start the request thread
	void Request::Start()
	{
		if (CheckStarted() || CheckDone())
		{
			return;
		}

		if (easy)
		{
#ifdef REQUEST_USE_CURL_MIMEPOST
			if (post_fields)
			{
				curl_easy_setopt(easy, CURLOPT_MIMEPOST, post_fields);
			}
			else
			{
				curl_easy_setopt(easy, CURLOPT_HTTPGET, 1L);
			}
#else
			if (!post_fields_map.empty())
			{
				for (auto &field : post_fields_map)
				{
					if (auto split = field.first.SplitBy(':'))
					{
						curl_formadd(&post_fields_first, &post_fields_last,
							CURLFORM_COPYNAME, split.Before().c_str(),
							CURLFORM_BUFFER, split.After().c_str(),
							CURLFORM_BUFFERPTR, &field.second[0],
							CURLFORM_BUFFERLENGTH, field.second.size(),
						CURLFORM_END);
					}
					else
					{
						curl_formadd(&post_fields_first, &post_fields_last,
							CURLFORM_COPYNAME, field.first.c_str(),
							CURLFORM_PTRCONTENTS, &field.second[0],
							CURLFORM_CONTENTLEN, field.second.size(),
						CURLFORM_END);
					}
				}
				curl_easy_setopt(easy, CURLOPT_HTTPPOST, post_fields_first);
			}
			else
			{
				curl_easy_setopt(easy, CURLOPT_HTTPGET, 1L);
			}
#endif

			curl_easy_setopt(easy, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(easy, CURLOPT_MAXREDIRS, 10L);

			curl_easy_setopt(easy, CURLOPT_ERRORBUFFER, error_buffer);
			error_buffer[0] = 0;

			curl_easy_setopt(easy, CURLOPT_TIMEOUT, timeout);
			curl_easy_setopt(easy, CURLOPT_HTTPHEADER, headers);
			curl_easy_setopt(easy, CURLOPT_URL, uri.c_str());

			if (proxy.size())
			{
				curl_easy_setopt(easy, CURLOPT_PROXY, proxy.c_str());
			}

			curl_easy_setopt(easy, CURLOPT_PRIVATE, (void *)this);
			curl_easy_setopt(easy, CURLOPT_USERAGENT, user_agent.c_str());
			curl_easy_setopt(easy, CURLOPT_NOSIGNAL, 1L);

			curl_easy_setopt(easy, CURLOPT_WRITEDATA, (void *)this);
			curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, Request::WriteDataHandler);
		}

		{
			std::lock_guard<std::mutex> g(rm_mutex);
			rm_started = true;
		}
		RequestManager::Ref().StartRequest(this);
	}


	// finish the request (if called before the request is done, this will block)
	ByteString Request::Finish(int *status_out)
	{
		if (CheckCanceled())
		{
			return ""; // shouldn't happen but just in case
		}
		
		ByteString response_out;
		{
			std::unique_lock<std::mutex> l(rm_mutex);
			done_cv.wait(l, [this]() { return rm_finished; });
			rm_started = false;
			rm_canceled = true;
			if (status_out)
			{
				*status_out = status;
			}
			response_out = std::move(response_body);
		}

		RequestManager::Ref().RemoveRequest(this);
		return response_out;
	}

	void Request::CheckProgress(int *total, int *done)
	{
		std::lock_guard<std::mutex> g(rm_mutex);
		if (total)
		{
			*total = rm_total;
		}
		if (done)
		{
			*done = rm_done;
		}
	}

	// returns true if the request has finished
	bool Request::CheckDone()
	{
		std::lock_guard<std::mutex> g(rm_mutex);
		return rm_finished;
	}

	// returns true if the request was canceled
	bool Request::CheckCanceled()
	{
		std::lock_guard<std::mutex> g(rm_mutex);
		return rm_canceled;
	}

	// returns true if the request is running
	bool Request::CheckStarted()
	{
		std::lock_guard<std::mutex> g(rm_mutex);
		return rm_started;

	}

	// cancels the request, the request thread will delete the Request* when it finishes (do not use Request in any way after canceling)
	void Request::Cancel()
	{
		{
			std::lock_guard<std::mutex> g(rm_mutex);
			rm_canceled = true;
		}
		RequestManager::Ref().RemoveRequest(this);
	}

	ByteString Request::Simple(ByteString uri, int *status, std::map<ByteString, ByteString> post_data)
	{
		return SimpleAuth(uri, status, "", "", post_data);
	}

	ByteString Request::SimpleAuth(ByteString uri, int *status, ByteString ID, ByteString session, std::map<ByteString, ByteString> post_data)
	{
		Request *request = new Request(uri);
		request->AddPostData(post_data);
		request->AuthHeaders(ID, session);
		request->Start();
		return request->Finish(status);
	}

	String StatusText(int ret)
	{
		switch (ret)
		{
		case 0:   return "Status code 0 (bug?)";
		case 100: return "Continue";
		case 101: return "Switching Protocols";
		case 102: return "Processing";
		case 200: return "OK";
		case 201: return "Created";
		case 202: return "Accepted";
		case 203: return "Non-Authoritative Information";
		case 204: return "No Content";
		case 205: return "Reset Content";
		case 206: return "Partial Content";
		case 207: return "Multi-Status";
		case 300: return "Multiple Choices";
		case 301: return "Moved Permanently";
		case 302: return "Found";
		case 303: return "See Other";
		case 304: return "Not Modified";
		case 305: return "Use Proxy";
		case 306: return "Switch Proxy";
		case 307: return "Temporary Redirect";
		case 400: return "Bad Request";
		case 401: return "Unauthorized";
		case 402: return "Payment Required";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 406: return "Not Acceptable";
		case 407: return "Proxy Authentication Required";
		case 408: return "Request Timeout";
		case 409: return "Conflict";
		case 410: return "Gone";
		case 411: return "Length Required";
		case 412: return "Precondition Failed";
		case 413: return "Request Entity Too Large";
		case 414: return "Request URI Too Long";
		case 415: return "Unsupported Media Type";
		case 416: return "Requested Range Not Satisfiable";
		case 417: return "Expectation Failed";
		case 418: return "I'm a teapot";
		case 422: return "Unprocessable Entity";
		case 423: return "Locked";
		case 424: return "Failed Dependency";
		case 425: return "Unordered Collection";
		case 426: return "Upgrade Required";
		case 444: return "No Response";
		case 450: return "Blocked by Windows Parental Controls";
		case 499: return "Client Closed Request";
		case 500: return "Internal Server Error";
		case 501: return "Not Implemented";
		case 502: return "Bad Gateway";
		case 503: return "Service Unavailable";
		case 504: return "Gateway Timeout";
		case 505: return "HTTP Version Not Supported";
		case 506: return "Variant Also Negotiates";
		case 507: return "Insufficient Storage";
		case 509: return "Bandwidth Limit Exceeded";
		case 510: return "Not Extended";
		case 600: return "Internal Client Error";
		case 601: return "Unsupported Protocol";
		case 602: return "Server Not Found";
		case 603: return "Malformed Response";
		case 604: return "Network Not Available";
		case 605: return "Request Timed Out";
		case 606: return "Malformed URL";
		case 607: return "Connection Refused";
		case 608: return "Proxy Server Not Found";
		case 609: return "SSL: Invalid Certificate Status";
		case 610: return "Cancelled by Shutdown";
		case 611: return "Too Many Redirects";
		case 612: return "SSL: Connect Error";
		case 613: return "SSL: Crypto Engine Not Found";
		case 614: return "SSL: Failed to Set Default Crypto Engine";
		case 615: return "SSL: Local Certificate Issue";
		case 616: return "SSL: Unable to Use Specified Cipher";
		case 617: return "SSL: Failed to Initialise Crypto Engine";
		case 618: return "SSL: Failed to Load CACERT File";
		case 619: return "SSL: Failed to Load CRL File";
		case 620: return "SSL: Issuer Check Failed";
		case 621: return "SSL: Pinned Public Key Mismatch";
		default:  return "Unknown Status Code";
		}
	}
}

