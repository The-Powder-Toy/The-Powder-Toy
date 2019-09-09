#include "Request.h"

#include "RequestManager.h"

#ifndef NOHTTP
void SetupCurlEasyCiphers(CURL *easy)
{
#ifdef SECURE_CIPHERS_ONLY
	curl_version_info_data *version_info = curl_version_info(CURLVERSION_NOW);
	ByteString ssl_type = version_info->ssl_version;
	if (ssl_type.Contains("OpenSSL"))
	{
		curl_easy_setopt(easy, CURLOPT_SSL_CIPHER_LIST, "ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-SHA384:DHE-RSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-CHACHA20-POLY1305:ECDHE-RSA-AES256-SHA384:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-ECDSA-AES256-SHA:ECDHE-RSA-AES128-SHA:DHE-RSA-AES128-GCM-SHA256");
#ifdef REQUEST_USE_CURL_TLSV13CL
		curl_easy_setopt(easy, CURLOPT_TLS13_CIPHERS, "TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_SHA256:TLS_AES_128_GCM_SHA256:TLS_AES_128_CCM_8_SHA256:TLS_AES_128_CCM_SHA256");
#endif
	}
	else if (ssl_type.Contains("Schannel"))
	{
		// TODO: add more cipher algorithms
		curl_easy_setopt(easy, CURLOPT_SSL_CIPHER_LIST, "CALG_ECDH_EPHEM");
	}
#endif
	// TODO: Find out what TLS1.2 is supported on, might need to also allow TLS1.0
	curl_easy_setopt(easy, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
#if defined(CURL_AT_LEAST_VERSION) && CURL_AT_LEAST_VERSION(7, 70, 0)
	curl_easy_setopt(easy, CURLOPT_SSL_OPTIONS, CURLSSLOPT_REVOKE_BEST_EFFORT);
#elif defined(CURL_AT_LEAST_VERSION) && CURL_AT_LEAST_VERSION(7, 44, 0)
	curl_easy_setopt(easy, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NO_REVOKE);
#elif defined(WIN)
# error "That's unfortunate."
#endif
}
#endif

namespace http
{
#ifndef NOHTTP
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
		if (!RequestManager::Ref().AddRequest(this))
		{
			status = 604;
			rm_finished = true;
		}
	}
#else
	Request::Request(ByteString uri_) {}
#endif

	Request::~Request()
	{
#ifndef NOHTTP
		curl_easy_cleanup(easy);
#ifdef REQUEST_USE_CURL_MIMEPOST
		curl_mime_free(post_fields);
#else
		curl_formfree(post_fields_first);
#endif
		curl_slist_free_all(headers);
#endif
	}

	void Request::AddHeader(ByteString name, ByteString value)
	{
#ifndef NOHTTP
		headers = curl_slist_append(headers, (name + ": " + value).c_str());
#endif
	}

	// add post data to a request
	void Request::AddPostData(std::map<ByteString, ByteString> data)
	{
#ifndef NOHTTP
		// Even if the map is empty, calling this function signifies you want to do a POST request
		isPost = true;
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
#endif
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

#ifndef NOHTTP
	size_t Request::WriteDataHandler(char *ptr, size_t size, size_t count, void *userdata)
	{
		Request *req = (Request *)userdata;
		auto actual_size = size * count;
		req->response_body.append(ptr, actual_size);
		return actual_size;
	}
#endif

	// start the request thread
	void Request::Start()
	{
#ifndef NOHTTP
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
#endif
			else if (isPost)
			{
				curl_easy_setopt(easy, CURLOPT_POST, 1L);
				curl_easy_setopt(easy, CURLOPT_POSTFIELDS, "");
			}
			else
			{
				curl_easy_setopt(easy, CURLOPT_HTTPGET, 1L);
			}

			curl_easy_setopt(easy, CURLOPT_FOLLOWLOCATION, 1L);
#ifdef ENFORCE_HTTPS
			curl_easy_setopt(easy, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
			curl_easy_setopt(easy, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTPS);
#else
			curl_easy_setopt(easy, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS | CURLPROTO_HTTP);
			curl_easy_setopt(easy, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTPS | CURLPROTO_HTTP);
#endif

			SetupCurlEasyCiphers(easy);

			curl_easy_setopt(easy, CURLOPT_MAXREDIRS, 10L);

			curl_easy_setopt(easy, CURLOPT_ERRORBUFFER, error_buffer);
			error_buffer[0] = 0;

			curl_easy_setopt(easy, CURLOPT_CONNECTTIMEOUT, timeout);
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
#endif
	}


	// finish the request (if called before the request is done, this will block)
	ByteString Request::Finish(int *status_out)
	{
#ifndef NOHTTP
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
#else
		if (status_out)
			*status_out = 604;
		return "";
#endif
	}

	void Request::CheckProgress(int *total, int *done)
	{
#ifndef NOHTTP
		std::lock_guard<std::mutex> g(rm_mutex);
		if (total)
		{
			*total = rm_total;
		}
		if (done)
		{
			*done = rm_done;
		}
#endif
	}

	// returns true if the request has finished
	bool Request::CheckDone()
	{
#ifndef NOHTTP
		std::lock_guard<std::mutex> g(rm_mutex);
		return rm_finished;
#else
		return true;
#endif
	}

	// returns true if the request was canceled
	bool Request::CheckCanceled()
	{
#ifndef NOHTTP
		std::lock_guard<std::mutex> g(rm_mutex);
		return rm_canceled;
#else
		return false;
#endif
	}

	// returns true if the request is running
	bool Request::CheckStarted()
	{
#ifndef NOHTTP
		std::lock_guard<std::mutex> g(rm_mutex);
		return rm_started;
#else
		return true;
#endif

	}

	// cancels the request, the request thread will delete the Request* when it finishes (do not use Request in any way after canceling)
	void Request::Cancel()
	{
#ifndef NOHTTP
		{
			std::lock_guard<std::mutex> g(rm_mutex);
			rm_canceled = true;
		}
		RequestManager::Ref().RemoveRequest(this);
#endif
	}

	ByteString Request::Simple(ByteString uri, int *status, std::map<ByteString, ByteString> post_data)
	{
		return SimpleAuth(uri, status, "", "", post_data);
	}

	ByteString Request::SimpleAuth(ByteString uri, int *status, ByteString ID, ByteString session, std::map<ByteString, ByteString> post_data)
	{
		Request *request = new Request(uri);
		if (!post_data.empty())
		{
			request->AddPostData(post_data);
		}
		request->AuthHeaders(ID, session);
		request->Start();
		return request->Finish(status);
	}

	String StatusText(int ret)
	{
		switch (ret)
		{
		case 0:   return String("Status code 0 (bug?)");
		case 100: return String("Continue");
		case 101: return String("Switching Protocols");
		case 102: return String("Processing");
		case 200: return String("OK");
		case 201: return String("Created");
		case 202: return String("Accepted");
		case 203: return String("Non-Authoritative Information");
		case 204: return String("No Content");
		case 205: return String("Reset Content");
		case 206: return String("Partial Content");
		case 207: return String("Multi-Status");
		case 300: return String("Multiple Choices");
		case 301: return String("Moved Permanently");
		case 302: return String("Found");
		case 303: return String("See Other");
		case 304: return String("Not Modified");
		case 305: return String("Use Proxy");
		case 306: return String("Switch Proxy");
		case 307: return String("Temporary Redirect");
		case 400: return String("Bad Request");
		case 401: return String("Unauthorized");
		case 402: return String("Payment Required");
		case 403: return String("Forbidden");
		case 404: return String("Not Found");
		case 405: return String("Method Not Allowed");
		case 406: return String("Not Acceptable");
		case 407: return String("Proxy Authentication Required");
		case 408: return String("Request Timeout");
		case 409: return String("Conflict");
		case 410: return String("Gone");
		case 411: return String("Length Required");
		case 412: return String("Precondition Failed");
		case 413: return String("Request Entity Too Large");
		case 414: return String("Request URI Too Long");
		case 415: return String("Unsupported Media Type");
		case 416: return String("Requested Range Not Satisfiable");
		case 417: return String("Expectation Failed");
		case 418: return String("I'm a teapot");
		case 422: return String("Unprocessable Entity");
		case 423: return String("Locked");
		case 424: return String("Failed Dependency");
		case 425: return String("Unordered Collection");
		case 426: return String("Upgrade Required");
		case 444: return String("No Response");
		case 450: return String("Blocked by Windows Parental Controls");
		case 499: return String("Client Closed Request");
		case 500: return String("Internal Server Error");
		case 501: return String("Not Implemented");
		case 502: return String("Bad Gateway");
		case 503: return String("Service Unavailable");
		case 504: return String("Gateway Timeout");
		case 505: return String("HTTP Version Not Supported");
		case 506: return String("Variant Also Negotiates");
		case 507: return String("Insufficient Storage");
		case 509: return String("Bandwidth Limit Exceeded");
		case 510: return String("Not Extended");
		case 600: return String("Internal Client Error");
		case 601: return String("Unsupported Protocol");
		case 602: return String("Server Not Found");
		case 603: return String("Malformed Response");
		case 604: return String("Network Not Available");
		case 605: return String("Request Timed Out");
		case 606: return String("Malformed URL");
		case 607: return String("Connection Refused");
		case 608: return String("Proxy Server Not Found");
		case 609: return String("SSL: Invalid Certificate Status");
		case 610: return String("Cancelled by Shutdown");
		case 611: return String("Too Many Redirects");
		case 612: return String("SSL: Connect Error");
		case 613: return String("SSL: Crypto Engine Not Found");
		case 614: return String("SSL: Failed to Set Default Crypto Engine");
		case 615: return String("SSL: Local Certificate Issue");
		case 616: return String("SSL: Unable to Use Specified Cipher");
		case 617: return String("SSL: Failed to Initialise Crypto Engine");
		case 618: return String("SSL: Failed to Load CACERT File");
		case 619: return String("SSL: Failed to Load CRL File");
		case 620: return String("SSL: Issuer Check Failed");
		case 621: return String("SSL: Pinned Public Key Mismatch");
		default:  return String("Unknown Status Code");
		}
	}
}

