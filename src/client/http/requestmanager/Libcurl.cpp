#include <curl/curl.h> // Has to come first because windows(tm).
#include "RequestManager.h"
#include "client/http/Request.h"
#include "CurlError.h"
#include "Config.h"
#include <iostream>

#if defined(CURL_AT_LEAST_VERSION) && CURL_AT_LEAST_VERSION(7, 55, 0)
# define REQUEST_USE_CURL_OFFSET_T
#endif
#if defined(CURL_AT_LEAST_VERSION) && CURL_AT_LEAST_VERSION(7, 56, 0)
# define REQUEST_USE_CURL_MIMEPOST
#endif
#if defined(CURL_AT_LEAST_VERSION) && CURL_AT_LEAST_VERSION(7, 61, 0)
# define REQUEST_USE_CURL_TLSV13CL
#endif

constexpr long curlMaxHostConnections   = 1;
constexpr long curlMaxConcurrentStreams = httpMaxConcurrentStreams;
constexpr long curlConnectTimeoutS      = httpConnectTimeoutS;

namespace http
{
	void HandleCURLcode(CURLcode code)
	{
		if (code != CURLE_OK)
		{
			throw CurlError(curl_easy_strerror(code));
		}
	};

	void HandleCURLMcode(CURLMcode code)
	{
		if (code != CURLM_OK && code != CURLM_CALL_MULTI_PERFORM)
		{
			throw CurlError(curl_multi_strerror(code));
		}
	};

#ifndef REQUEST_USE_CURL_MIMEPOST
	void HandleCURLFORMcode(CURLFORMcode code)
	{
		if (code != CURL_FORMADD_OK)
		{
			throw CurlError(ByteString::Build("CURLFORMcode ", code));
		}
	};
#endif

	struct RequestHandleHttp : public RequestHandle
	{
		curl_slist *curlHeaders = NULL;
#ifdef REQUEST_USE_CURL_MIMEPOST
		curl_mime *curlPostFields = NULL;
#else
		curl_httppost *curlPostFieldsFirst = NULL;
		curl_httppost *curlPostFieldsLast = NULL;
#endif
		CURL *curlEasy = NULL;
		char curlErrorBuffer[CURL_ERROR_SIZE];
		bool curlAddedToMulti = false;
		bool gotStatusLine = false;

		RequestHandleHttp() : RequestHandle(CtorTag{})
		{
		}

		static size_t HeaderDataHandler(char *ptr, size_t size, size_t count, void *userdata)
		{
			auto *handle = (RequestHandleHttp *)userdata;
			auto bytes = size * count;
			if (bytes >= 2 && ptr[bytes - 2] == '\r' && ptr[bytes - 1] == '\n')
			{
				if (bytes > 2 && handle->gotStatusLine) // Don't include header list terminator or the status line.
				{
					auto line = ByteString(ptr, ptr + bytes - 2);
					if (auto split = line.SplitBy(':'))
					{
						auto value = split.After();
						while (value.size() && (value.front() == ' ' || value.front() == '\t'))
						{
							value = value.Substr(1);
						}
						while (value.size() && (value.back() == ' ' || value.back() == '\t'))
						{
							value = value.Substr(0, value.size() - 1);
						}
						handle->responseHeaders.push_back({ split.Before().ToLower(), value });
					}
					else
					{
						std::cerr << "skipping weird header: " << line << std::endl;
					}
				}
				handle->gotStatusLine = true;
				return bytes;
			}
			return 0;
		}

		static size_t WriteDataHandler(char *ptr, size_t size, size_t count, void *userdata)
		{
			auto *handle = (RequestHandleHttp *)userdata;
			auto bytes = size * count;
			handle->responseData.append(ptr, bytes);
			return bytes;
		}
	};

	std::shared_ptr<RequestHandle> RequestHandle::Create()
	{
		return std::make_shared<RequestHandleHttp>();
	}

	struct RequestManagerImpl : public RequestManager
	{
		using RequestManager::RequestManager;

		RequestManagerImpl(ByteString newProxy, ByteString newCafile, ByteString newCapath, bool newDisableNetwork);
		~RequestManagerImpl();

		std::thread worker;
		void Worker();
		void WorkerInit();
		void WorkerPerform();
		void WorkerExit();

		// State shared between Request threads and the worker thread.
		std::vector<std::shared_ptr<RequestHandle>> requestHandlesToRegister;
		std::vector<std::shared_ptr<RequestHandle>> requestHandlesToUnregister;
		bool running = true;
		std::mutex sharedStateMx;

		std::vector<std::shared_ptr<RequestHandle>> requestHandles;
		void RegisterRequestHandle(std::shared_ptr<RequestHandle> requestHandle);
		void UnregisterRequestHandle(std::shared_ptr<RequestHandle> requestHandle);

		bool curlGlobalInit = false;
		CURLM *curlMulti = NULL;
	};

	RequestManagerImpl::RequestManagerImpl(ByteString newProxy, ByteString newCafile, ByteString newCapath, bool newDisableNetwork) :
		RequestManager(newProxy, newCafile, newCapath, newDisableNetwork)
	{
		worker = std::thread([this]() {
			Worker();
		});
	}

	RequestManagerImpl::~RequestManagerImpl()
	{
		{
			std::lock_guard lk(sharedStateMx);
			running = false;
		}
		worker.join();
	}

	void RequestManagerImpl::WorkerInit()
	{
		if (!curl_global_init(CURL_GLOBAL_DEFAULT))
		{
			curlGlobalInit = true;
			curlMulti = curl_multi_init();
			if (curlMulti)
			{
				HandleCURLMcode(curl_multi_setopt(curlMulti, CURLMOPT_MAX_HOST_CONNECTIONS, curlMaxHostConnections));
#if defined(CURL_AT_LEAST_VERSION) && CURL_AT_LEAST_VERSION(7, 67, 0)
				HandleCURLMcode(curl_multi_setopt(curlMulti, CURLMOPT_MAX_CONCURRENT_STREAMS, curlMaxConcurrentStreams));
#endif
			}
		}
	}

	void RequestManagerImpl::WorkerPerform()
	{
		auto manager = static_cast<RequestManagerImpl *>(this);
		int dontcare;
		HandleCURLMcode(curl_multi_poll(manager->curlMulti, NULL, 0, 1000, &dontcare));
		HandleCURLMcode(curl_multi_perform(manager->curlMulti, &dontcare));
		while (auto msg = curl_multi_info_read(manager->curlMulti, &dontcare))
		{
			if (msg->msg == CURLMSG_DONE)
			{
				RequestHandleHttp *handle;
				HandleCURLcode(curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &handle));
				handle->statusCode = 600;
				switch (msg->data.result)
				{
				case CURLE_OK:
					{
						long code;
						HandleCURLcode(curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &code));
						assert(code);
						handle->statusCode = int(code);
					}
					break;
				
				case CURLE_UNSUPPORTED_PROTOCOL:     handle->statusCode = 601; break;
				case CURLE_COULDNT_RESOLVE_HOST:     handle->statusCode = 602; break;
				case CURLE_OPERATION_TIMEDOUT:       handle->statusCode = 605; break;
				case CURLE_URL_MALFORMAT:            handle->statusCode = 606; break;
				case CURLE_COULDNT_CONNECT:          handle->statusCode = 607; break;
				case CURLE_COULDNT_RESOLVE_PROXY:    handle->statusCode = 608; break;
				case CURLE_TOO_MANY_REDIRECTS:       handle->statusCode = 611; break;
				case CURLE_SSL_CONNECT_ERROR:        handle->statusCode = 612; break;
				case CURLE_SSL_ENGINE_NOTFOUND:      handle->statusCode = 613; break;
				case CURLE_SSL_ENGINE_SETFAILED:     handle->statusCode = 614; break;
				case CURLE_SSL_CERTPROBLEM:          handle->statusCode = 615; break;
				case CURLE_SSL_CIPHER:               handle->statusCode = 616; break;
				case CURLE_SSL_ENGINE_INITFAILED:    handle->statusCode = 617; break;
				case CURLE_SSL_CACERT_BADFILE:       handle->statusCode = 618; break;
				case CURLE_SSL_CRL_BADFILE:          handle->statusCode = 619; break;
				case CURLE_SSL_ISSUER_ERROR:         handle->statusCode = 620; break;
				case CURLE_SSL_PINNEDPUBKEYNOTMATCH: handle->statusCode = 621; break;
				case CURLE_SSL_INVALIDCERTSTATUS:    handle->statusCode = 609; break;
				case CURLE_HTTP2:
				case CURLE_HTTP2_STREAM:
				case CURLE_FAILED_INIT:
				case CURLE_NOT_BUILT_IN:
				default:
					break;
				}
				if (handle->statusCode >= 600)
				{
					handle->error = handle->curlErrorBuffer;
				}
			}
		}
		for (auto &requestHandle : requestHandles)
		{
			auto handle = static_cast<RequestHandleHttp *>(requestHandle.get());
			if (handle->curlEasy)
			{
#ifdef REQUEST_USE_CURL_OFFSET_T
				curl_off_t total, done;
				HandleCURLcode(curl_easy_getinfo(handle->curlEasy, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &total)); // stores -1 if unknown
				HandleCURLcode(curl_easy_getinfo(handle->curlEasy, CURLINFO_SIZE_DOWNLOAD_T, &done));
#else
				double total, done;
				HandleCURLcode(curl_easy_getinfo(handle->curlEasy, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &total)); // stores -1 if unknown
				HandleCURLcode(curl_easy_getinfo(handle->curlEasy, CURLINFO_SIZE_DOWNLOAD, &done));
#endif
				handle->bytesTotal = int64_t(total);
				handle->bytesDone = int64_t(done);
			}
			else
			{
				handle->bytesTotal = -1;
				handle->bytesDone = 0;
			}
		}
	}

	void RequestManagerImpl::WorkerExit()
	{
		curl_multi_cleanup(curlMulti);
		curlMulti = NULL;
		curl_global_cleanup();
	}

	void RequestManagerImpl::Worker()
	{
		WorkerInit();
		while (true)
		{
			{
				std::lock_guard lk(sharedStateMx);
				for (auto &requestHandle : requestHandles)
				{
					if (requestHandle->statusCode)
					{
						requestHandlesToUnregister.push_back(requestHandle);
					}
				}
				for (auto &requestHandle : requestHandlesToRegister)
				{
					// Must not be present
					assert(std::find(requestHandles.begin(), requestHandles.end(), requestHandle) == requestHandles.end());
					requestHandles.push_back(requestHandle);
					RegisterRequestHandle(requestHandle);
				}
				requestHandlesToRegister.clear();
				for (auto &requestHandle : requestHandlesToUnregister)
				{
					auto eraseFrom = std::remove(requestHandles.begin(), requestHandles.end(), requestHandle);
					// Must either not be present
					if (eraseFrom != requestHandles.end())
					{
						// Or be present exactly once
						assert(eraseFrom + 1 == requestHandles.end());
						UnregisterRequestHandle(requestHandle);
						requestHandles.erase(eraseFrom, requestHandles.end());
						requestHandle->MarkDone();
					}
				}
				requestHandlesToUnregister.clear();
				if (!running)
				{
					break;
				}
			}
			WorkerPerform();
		}
		assert(!requestHandles.size());
		WorkerExit();
	}

	void RequestManager::RegisterRequestImpl(Request &request)
	{
		auto manager = static_cast<RequestManagerImpl *>(this);
		{
			std::lock_guard lk(manager->sharedStateMx);
			manager->requestHandlesToRegister.push_back(request.handle);
		}
		curl_multi_wakeup(manager->curlMulti);
	}

	void RequestManager::UnregisterRequestImpl(Request &request)
	{
		auto manager = static_cast<RequestManagerImpl *>(this);
		{
			std::lock_guard lk(manager->sharedStateMx);
			manager->requestHandlesToUnregister.push_back(request.handle);
		}
		curl_multi_wakeup(manager->curlMulti);
	}

	void RequestManagerImpl::RegisterRequestHandle(std::shared_ptr<RequestHandle> requestHandle)
	{
		auto manager = static_cast<RequestManagerImpl *>(this);
		auto handle = static_cast<RequestHandleHttp *>(requestHandle.get());
		auto failEarly = [&requestHandle](int statusCode, ByteString error) {
			requestHandle->statusCode = statusCode;
			requestHandle->error = error;
		};
		if (!manager->curlGlobalInit)
		{
			return failEarly(600, "no CURL");
		}
		if (!manager->curlMulti)
		{
			return failEarly(600, "no CURL multi handle");
		}
		try
		{
			handle->curlEasy = curl_easy_init();
			if (!handle->curlEasy)
			{
				return failEarly(600, "no CURL easy handle");
			} 
			for (auto &header : handle->headers)
			{
				auto *newHeaders = curl_slist_append(handle->curlHeaders, (header.name + ": " + header.value).c_str());
				if (!newHeaders)
				{
					// Hopefully this is what a NULL from curl_slist_append means.
					HandleCURLcode(CURLE_OUT_OF_MEMORY);
				}
				handle->curlHeaders = newHeaders;
			}
			{
				auto &postData = handle->postData;
				if (std::holds_alternative<http::FormData>(postData) && std::get<http::FormData>(postData).size())
				{
					auto &formData = std::get<http::FormData>(postData);
#ifdef REQUEST_USE_CURL_MIMEPOST
					handle->curlPostFields = curl_mime_init(handle->curlEasy);
					if (!handle->curlPostFields)
					{
						// Hopefully this is what a NULL from curl_mime_init means.
						HandleCURLcode(CURLE_OUT_OF_MEMORY);
					}
					for (auto &field : formData)
					{
						curl_mimepart *part = curl_mime_addpart(handle->curlPostFields);
						if (!part)
						{
							// Hopefully this is what a NULL from curl_mime_addpart means.
							HandleCURLcode(CURLE_OUT_OF_MEMORY);
						}
						HandleCURLcode(curl_mime_data(part, &field.value[0], field.value.size()));
						HandleCURLcode(curl_mime_name(part, field.name.c_str()));
						if (field.filename.has_value())
						{
							HandleCURLcode(curl_mime_filename(part, field.filename->c_str()));
						}
					}
					HandleCURLcode(curl_easy_setopt(handle->curlEasy, CURLOPT_MIMEPOST, handle->curlPostFields));
#else
					for (auto &field : formData)
					{
						if (field.filename.has_value())
						{
							HandleCURLFORMcode(curl_formadd(&handle->curlPostFieldsFirst, &handle->curlPostFieldsLast,
								CURLFORM_COPYNAME, field.name.c_str(),
								CURLFORM_BUFFER, field.filename->c_str(),
								CURLFORM_BUFFERPTR, &field.value[0],
								CURLFORM_BUFFERLENGTH, field.value.size(),
							CURLFORM_END));
						}
						else
						{
							HandleCURLFORMcode(curl_formadd(&handle->curlPostFieldsFirst, &handle->curlPostFieldsLast,
								CURLFORM_COPYNAME, field.name.c_str(),
								CURLFORM_PTRCONTENTS, &field.value[0],
								CURLFORM_CONTENTLEN, field.value.size(),
							CURLFORM_END));
						}
					}
					HandleCURLcode(curl_easy_setopt(handle->curlEasy, CURLOPT_HTTPPOST, handle->curlPostFieldsFirst));
#endif
				}
				else if (std::holds_alternative<http::StringData>(postData) && std::get<http::StringData>(postData).size())
				{
					auto &stringData = std::get<http::StringData>(postData);
					HandleCURLcode(curl_easy_setopt(handle->curlEasy, CURLOPT_POSTFIELDS, &stringData[0]));
					HandleCURLcode(curl_easy_setopt(handle->curlEasy, CURLOPT_POSTFIELDSIZE_LARGE, curl_off_t(stringData.size())));
				}
				else if (handle->isPost)
				{
					HandleCURLcode(curl_easy_setopt(handle->curlEasy, CURLOPT_POST, 1L));
					HandleCURLcode(curl_easy_setopt(handle->curlEasy, CURLOPT_POSTFIELDS, ""));
				}
				else
				{
					HandleCURLcode(curl_easy_setopt(handle->curlEasy, CURLOPT_HTTPGET, 1L));
				}
				if (handle->verb)
				{
					HandleCURLcode(curl_easy_setopt(handle->curlEasy, CURLOPT_CUSTOMREQUEST, handle->verb->c_str()));
				}
				HandleCURLcode(curl_easy_setopt(handle->curlEasy, CURLOPT_FOLLOWLOCATION, 1L));
				if constexpr (ENFORCE_HTTPS)
				{
#if defined(CURL_AT_LEAST_VERSION) && CURL_AT_LEAST_VERSION(7, 85, 0)
					HandleCURLcode(curl_easy_setopt(handle->curlEasy, CURLOPT_PROTOCOLS_STR, "https"));
					HandleCURLcode(curl_easy_setopt(handle->curlEasy, CURLOPT_REDIR_PROTOCOLS_STR, "https"));
#else
					HandleCURLcode(curl_easy_setopt(handle->curlEasy, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS));
					HandleCURLcode(curl_easy_setopt(handle->curlEasy, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTPS));
#endif
				}
				else
				{
#if defined(CURL_AT_LEAST_VERSION) && CURL_AT_LEAST_VERSION(7, 85, 0)
					HandleCURLcode(curl_easy_setopt(handle->curlEasy, CURLOPT_PROTOCOLS_STR, "https,http"));
					HandleCURLcode(curl_easy_setopt(handle->curlEasy, CURLOPT_REDIR_PROTOCOLS_STR, "https,http"));
#else
					HandleCURLcode(curl_easy_setopt(handle->curlEasy, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS | CURLPROTO_HTTP));
					HandleCURLcode(curl_easy_setopt(handle->curlEasy, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTPS | CURLPROTO_HTTP));
#endif
				}
				SetupCurlEasyCiphers(handle->curlEasy);
				HandleCURLcode(curl_easy_setopt(handle->curlEasy, CURLOPT_MAXREDIRS, 10L));
				HandleCURLcode(curl_easy_setopt(handle->curlEasy, CURLOPT_ERRORBUFFER, handle->curlErrorBuffer));
				handle->curlErrorBuffer[0] = 0;
				HandleCURLcode(curl_easy_setopt(handle->curlEasy, CURLOPT_CONNECTTIMEOUT, curlConnectTimeoutS));
				HandleCURLcode(curl_easy_setopt(handle->curlEasy, CURLOPT_HTTPHEADER, handle->curlHeaders));
				HandleCURLcode(curl_easy_setopt(handle->curlEasy, CURLOPT_URL, handle->uri.c_str()));
				if (proxy.size())
				{
					HandleCURLcode(curl_easy_setopt(handle->curlEasy, CURLOPT_PROXY, proxy.c_str()));
				}
				if (cafile.size())
				{
					HandleCURLcode(curl_easy_setopt(handle->curlEasy, CURLOPT_CAINFO, cafile.c_str()));
				}
				if (capath.size())
				{
					HandleCURLcode(curl_easy_setopt(handle->curlEasy, CURLOPT_CAPATH, capath.c_str()));
				}
				HandleCURLcode(curl_easy_setopt(handle->curlEasy, CURLOPT_PRIVATE, (void *)handle));
				HandleCURLcode(curl_easy_setopt(handle->curlEasy, CURLOPT_USERAGENT, userAgent.c_str()));
				HandleCURLcode(curl_easy_setopt(handle->curlEasy, CURLOPT_HEADERDATA, (void *)handle));
				HandleCURLcode(curl_easy_setopt(handle->curlEasy, CURLOPT_HEADERFUNCTION, &RequestHandleHttp::HeaderDataHandler));
				HandleCURLcode(curl_easy_setopt(handle->curlEasy, CURLOPT_WRITEDATA, (void *)handle));
				HandleCURLcode(curl_easy_setopt(handle->curlEasy, CURLOPT_WRITEFUNCTION, &RequestHandleHttp::WriteDataHandler));
			}
		}
		catch (const CurlError &ex)
		{
			return failEarly(600, ex.what());
		}
		HandleCURLMcode(curl_multi_add_handle(manager->curlMulti, handle->curlEasy));
		handle->curlAddedToMulti = true;
	}

	void RequestManagerImpl::UnregisterRequestHandle(std::shared_ptr<RequestHandle> requestHandle)
	{
		auto manager = static_cast<RequestManagerImpl *>(this);
		auto handle = static_cast<RequestHandleHttp *>(requestHandle.get());
		if (handle->curlAddedToMulti)
		{
			HandleCURLMcode(curl_multi_remove_handle(manager->curlMulti, handle->curlEasy));
			handle->curlAddedToMulti = false;
		}
		curl_easy_cleanup(handle->curlEasy);
#ifdef REQUEST_USE_CURL_MIMEPOST
		curl_mime_free(handle->curlPostFields);
#else
		curl_formfree(handle->curlPostFieldsFirst);
#endif
		curl_slist_free_all(handle->curlHeaders);
	}

	RequestManagerPtr RequestManager::Create(ByteString newProxy, ByteString newCafile, ByteString newCapath, bool newDisableNetwork)
	{
		return RequestManagerPtr(new RequestManagerImpl(newProxy, newCafile, newCapath, newDisableNetwork));
	}

	void RequestManagerDeleter::operator ()(RequestManager *ptr) const
	{
		delete static_cast<RequestManagerImpl *>(ptr);
	}

	void SetupCurlEasyCiphers(CURL *easy)
	{
		if constexpr (SECURE_CIPHERS_ONLY)
		{
			curl_version_info_data *version_info = curl_version_info(CURLVERSION_NOW);
			ByteString ssl_type = version_info->ssl_version;
			if (ssl_type.Contains("OpenSSL") || ssl_type.Contains("mbedTLS"))
			{
				HandleCURLcode(curl_easy_setopt(easy, CURLOPT_SSL_CIPHER_LIST,
					"ECDHE-ECDSA-AES256-GCM-SHA384" ":"
					"ECDHE-ECDSA-AES128-GCM-SHA256" ":"
					"ECDHE-ECDSA-AES256-SHA384"     ":"
					"DHE-RSA-AES256-GCM-SHA384"     ":"
					"ECDHE-RSA-AES256-GCM-SHA384"   ":"
					"ECDHE-RSA-AES128-GCM-SHA256"   ":"
					"ECDHE-ECDSA-AES128-SHA"        ":"
					"ECDHE-ECDSA-AES128-SHA256"     ":"
					"ECDHE-RSA-CHACHA20-POLY1305"   ":"
					"ECDHE-RSA-AES256-SHA384"       ":"
					"ECDHE-RSA-AES128-SHA256"       ":"
					"ECDHE-ECDSA-CHACHA20-POLY1305" ":"
					"ECDHE-ECDSA-AES256-SHA"        ":"
					"ECDHE-RSA-AES128-SHA"          ":"
					"DHE-RSA-AES128-GCM-SHA256"
				));
#ifdef REQUEST_USE_CURL_TLSV13CL
				HandleCURLcode(curl_easy_setopt(easy, CURLOPT_TLS13_CIPHERS,
					"TLS_AES_256_GCM_SHA384"       ":"
					"TLS_CHACHA20_POLY1305_SHA256" ":"
					"TLS_AES_128_GCM_SHA256"       ":"
					"TLS_AES_128_CCM_8_SHA256"     ":"
					"TLS_AES_128_CCM_SHA256"
				));
#endif
			}
			else if (ssl_type.Contains("Schannel"))
			{
				// TODO: add more cipher algorithms
				HandleCURLcode(curl_easy_setopt(easy, CURLOPT_SSL_CIPHER_LIST, "CALG_ECDH_EPHEM"));
			}
		}
		// TODO: Find out what TLS1.2 is supported on, might need to also allow TLS1.0
		HandleCURLcode(curl_easy_setopt(easy, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2));
#if defined(CURL_AT_LEAST_VERSION) && CURL_AT_LEAST_VERSION(7, 70, 0)
		HandleCURLcode(curl_easy_setopt(easy, CURLOPT_SSL_OPTIONS, CURLSSLOPT_REVOKE_BEST_EFFORT));
#elif defined(CURL_AT_LEAST_VERSION) && CURL_AT_LEAST_VERSION(7, 44, 0)
		HandleCURLcode(curl_easy_setopt(easy, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NO_REVOKE));
#endif
	}
}
