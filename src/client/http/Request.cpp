#include "Request.h"
#include "requestmanager/RequestManager.h"
#include <memory>
#include <iostream>
#include <cstring>
#include <json/json.h>

namespace http
{
	Request::Request(ByteString newUri)
	{
		handle = RequestHandle::Create();
		handle->uri = newUri;
	}

	Request::~Request()
	{
		bool tryUnregister;
		{
			std::lock_guard lk(handle->stateMx);
			tryUnregister = handle->state == RequestHandle::running;
		}
		if (tryUnregister)
		{
			// At this point it may have already finished and been unregistered but that's ok,
			// attempting to unregister a request multiple times is allowed. We only do the
			// state-checking dance so we don't wake up RequestManager if we don't have to.
			// In fact, we could just not unregister requests here at all, they'd just run to
			// completion and be unregistered later. All this does is cancel them early.
			RequestManager::Ref().UnregisterRequest(*this);
		}
	}

	void Request::FailEarly(ByteString error)
	{
		assert(handle->state == RequestHandle::ready);
		handle->failEarly = error;
	}

	void Request::Verb(ByteString newVerb)
	{
		assert(handle->state == RequestHandle::ready);
		handle->verb = newVerb;
	}

	void Request::AddHeader(ByteString header)
	{
		assert(handle->state == RequestHandle::ready);
		handle->headers.push_back(header);
	}

	void Request::AddPostData(PostData data)
	{
		assert(handle->state == RequestHandle::ready);
		// Even if the map is empty, calling this function signifies you want to do a POST request
		handle->isPost = true;
		handle->postData = data;
	}

	void Request::AuthHeaders(ByteString ID, ByteString session)
	{
		assert(handle->state == RequestHandle::ready);
		if (ID.size())
		{
			if (session.size())
			{
				AddHeader("X-Auth-User-Id: " + ID);
				AddHeader("X-Auth-Session-Key: " + session);
			}
			else
			{
				AddHeader("X-Auth-User: " + ID);
			}
		}
	}

	void Request::Start()
	{
		assert(handle->state == RequestHandle::ready);
		handle->state = RequestHandle::running;
		RequestManager::Ref().RegisterRequest(*this);
	}

	bool Request::CheckDone() const
	{
		std::lock_guard lk(handle->stateMx);
		assert(handle->state == RequestHandle::running || handle->state == RequestHandle::done);
		return handle->state == RequestHandle::done;
	}

	std::pair<int, int> Request::CheckProgress() const
	{
		std::lock_guard lk(handle->stateMx);
		assert(handle->state == RequestHandle::running || handle->state == RequestHandle::done);
		return { handle->bytesTotal, handle->bytesDone };
	}

	const std::vector<ByteString> &Request::ResponseHeaders() const
	{
		std::lock_guard lk(handle->stateMx);
		assert(handle->state == RequestHandle::done);
		return handle->responseHeaders;
	}

	void Request::Wait()
	{
		std::unique_lock lk(handle->stateMx);
		assert(handle->state == RequestHandle::running);
		handle->stateCv.wait(lk, [this]() {
			return handle->state == RequestHandle::done;
		});
	}

	int Request::StatusCode() const
	{
		{
			std::unique_lock lk(handle->stateMx);
			assert(handle->state == RequestHandle::done);
		}
		return handle->statusCode;
	}

	std::pair<int, ByteString> Request::Finish()
	{
		{
			std::unique_lock lk(handle->stateMx);
			assert(handle->state == RequestHandle::done);
		}
		handle->state = RequestHandle::dead;
		if (handle->error)
		{
			throw RequestError(*handle->error);
		}
		return std::pair{ handle->statusCode, std::move(handle->responseData) };
	}

	void RequestHandle::MarkDone()
	{
		{
			std::lock_guard lk(stateMx);
			assert(state == RequestHandle::running);
			state = RequestHandle::done;
		}
		stateCv.notify_one();
		if (error)
		{
			std::cerr << *error << std::endl;
		}
	}

	const char *StatusText(int ret)
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
		}
		return "Unknown Status Code";
	}

	void Request::ParseResponse(const ByteString &result, int status, ResponseType responseType)
	{
		// no server response, return "Malformed Response"
		if (status == 200 && !result.size())
		{
			status = 603;
		}
		if (status == 302)
		{
			return;
		}
		if (status != 200)
		{
			throw RequestError(ByteString::Build("HTTP Error ", status, ": ", http::StatusText(status)));
		}

		switch (responseType)
		{
		case responseOk:
			if (strncmp(result.c_str(), "OK", 2))
			{
				throw RequestError(result);
			}
			break;

		case responseJson:
			{
				std::istringstream ss(result);
				Json::Value root;
				try
				{
					ss >> root;
					// assume everything is fine if an empty [] is returned
					if (root.size() == 0)
					{
						return;
					}
					int status = root.get("Status", 1).asInt();
					if (status != 1)
					{
						throw RequestError(ByteString(root.get("Error", "Unspecified Error").asString()));
					}
				}
				catch (const std::exception &ex)
				{
					// sometimes the server returns a 200 with the text "Error: 401"
					if (!strncmp(result.c_str(), "Error: ", 7))
					{
						status = ByteString(result.begin() + 7, result.end()).ToNumber<int>();
						throw RequestError(ByteString::Build("HTTP Error ", status, ": ", http::StatusText(status)));
					}
					throw RequestError("Could not read response: " + ByteString(ex.what()));
				}
			}
			break;

		case responseData:
			// no further processing required
			break;
		}
	}
}
