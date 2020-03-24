#include "Request.h"

#include "RequestManager.h"

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

#ifdef SECURE_CIPHERS_ONLY
			curl_version_info_data* version_info = curl_version_info(CURLVERSION_NOW);
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
		case 0:   return "Status code 0 (bug?)"_i18n;
		case 100: return "Continue"_i18n;
		case 101: return "Switching Protocols"_i18n;
		case 102: return "Processing"_i18n;
		case 200: return "OK"_i18n;
		case 201: return "Created"_i18n;
		case 202: return "Accepted"_i18n;
		case 203: return "Non-Authoritative Information"_i18n;
		case 204: return "No Content"_i18n;
		case 205: return "Reset Content"_i18n;
		case 206: return "Partial Content"_i18n;
		case 207: return "Multi-Status"_i18n;
		case 300: return "Multiple Choices"_i18n;
		case 301: return "Moved Permanently"_i18n;
		case 302: return "Found"_i18n;
		case 303: return "See Other"_i18n;
		case 304: return "Not Modified"_i18n;
		case 305: return "Use Proxy"_i18n;
		case 306: return "Switch Proxy"_i18n;
		case 307: return "Temporary Redirect"_i18n;
		case 400: return "Bad Request"_i18n;
		case 401: return "Unauthorized"_i18n;
		case 402: return "Payment Required"_i18n;
		case 403: return "Forbidden"_i18n;
		case 404: return "Not Found"_i18n;
		case 405: return "Method Not Allowed"_i18n;
		case 406: return "Not Acceptable"_i18n;
		case 407: return "Proxy Authentication Required"_i18n;
		case 408: return "Request Timeout"_i18n;
		case 409: return "Conflict"_i18n;
		case 410: return "Gone"_i18n;
		case 411: return "Length Required"_i18n;
		case 412: return "Precondition Failed"_i18n;
		case 413: return "Request Entity Too Large"_i18n;
		case 414: return "Request URI Too Long"_i18n;
		case 415: return "Unsupported Media Type"_i18n;
		case 416: return "Requested Range Not Satisfiable"_i18n;
		case 417: return "Expectation Failed"_i18n;
		case 418: return "I'm a teapot"_i18n;
		case 422: return "Unprocessable Entity"_i18n;
		case 423: return "Locked"_i18n;
		case 424: return "Failed Dependency"_i18n;
		case 425: return "Unordered Collection"_i18n;
		case 426: return "Upgrade Required"_i18n;
		case 444: return "No Response"_i18n;
		case 450: return "Blocked by Windows Parental Controls"_i18n;
		case 499: return "Client Closed Request"_i18n;
		case 500: return "Internal Server Error"_i18n;
		case 501: return "Not Implemented"_i18n;
		case 502: return "Bad Gateway"_i18n;
		case 503: return "Service Unavailable"_i18n;
		case 504: return "Gateway Timeout"_i18n;
		case 505: return "HTTP Version Not Supported"_i18n;
		case 506: return "Variant Also Negotiates"_i18n;
		case 507: return "Insufficient Storage"_i18n;
		case 509: return "Bandwidth Limit Exceeded"_i18n;
		case 510: return "Not Extended"_i18n;
		case 600: return "Internal Client Error"_i18n;
		case 601: return "Unsupported Protocol"_i18n;
		case 602: return "Server Not Found"_i18n;
		case 603: return "Malformed Response"_i18n;
		case 604: return "Network Not Available"_i18n;
		case 605: return "Request Timed Out"_i18n;
		case 606: return "Malformed URL"_i18n;
		case 607: return "Connection Refused"_i18n;
		case 608: return "Proxy Server Not Found"_i18n;
		case 609: return "SSL: Invalid Certificate Status"_i18n;
		case 610: return "Cancelled by Shutdown"_i18n;
		case 611: return "Too Many Redirects"_i18n;
		case 612: return "SSL: Connect Error"_i18n;
		case 613: return "SSL: Crypto Engine Not Found"_i18n;
		case 614: return "SSL: Failed to Set Default Crypto Engine"_i18n;
		case 615: return "SSL: Local Certificate Issue"_i18n;
		case 616: return "SSL: Unable to Use Specified Cipher"_i18n;
		case 617: return "SSL: Failed to Initialise Crypto Engine"_i18n;
		case 618: return "SSL: Failed to Load CACERT File"_i18n;
		case 619: return "SSL: Failed to Load CRL File"_i18n;
		case 620: return "SSL: Issuer Check Failed"_i18n;
		case 621: return "SSL: Pinned Public Key Mismatch"_i18n;
		default:  return "Unknown Status Code"_i18n;
		}
	}
}

