#include "RequestManager.h"
#include "client/http/Request.h"
#include <algorithm>
#include <emscripten.h>
#include <emscripten/threading.h>

namespace http
{
	struct RequestHandleHttp : public RequestHandle
	{
		RequestHandleHttp() : RequestHandle(CtorTag{})
		{
		}

		bool gotResponse = false;
		int id = -1;
	};
}

EMSCRIPTEN_KEEPALIVE extern "C" void RequestManager_UpdateRequestStatusThunk(http::RequestHandleHttp *handle);

namespace http
{
	std::shared_ptr<RequestHandle> RequestHandle::Create()
	{
		return std::make_shared<RequestHandleHttp>();
	}

	struct RequestManagerImpl : public RequestManager
	{
		using RequestManager::RequestManager;

		RequestManagerImpl(ByteString newProxy, ByteString newCafile, ByteString newCapath, bool newDisableNetwork);
		~RequestManagerImpl();

		// State shared between Request threads and the worker thread.
		std::vector<std::shared_ptr<RequestHandle>> requestHandlesToRegister;
		std::vector<std::shared_ptr<RequestHandle>> requestHandlesToUnregister;
		std::mutex sharedStateMx;

		std::vector<std::shared_ptr<RequestHandle>> requestHandles;
		void RegisterRequestHandle(std::shared_ptr<RequestHandle> requestHandle);
		void UnregisterRequestHandle(std::shared_ptr<RequestHandle> requestHandle);

		void HandleWake();
		void Wake();

		void UpdateRequestStatus(RequestHandleHttp *handle);
	};

	RequestManagerImpl::RequestManagerImpl(ByteString newProxy, ByteString newCafile, ByteString newCapath, bool newDisableNetwork) :
		RequestManager(newProxy, newCafile, newCapath, newDisableNetwork)
	{
		EM_ASM({
			Module.emscriptenRequestManager = {};
			Module.emscriptenRequestManager.requests = [];
			Module.emscriptenRequestManager.updateRequestStatusThunk = Module.cwrap(
				'RequestManager_UpdateRequestStatusThunk',
				null,
				[ 'number' ]
			);
		});
	}

	RequestManagerImpl::~RequestManagerImpl()
	{
		// Nothing, we never really exit.
	}

	void RequestManager::RegisterRequestImpl(Request &request)
	{
		auto manager = static_cast<RequestManagerImpl *>(this);
		{
			std::lock_guard lk(manager->sharedStateMx);
			manager->requestHandlesToRegister.push_back(request.handle);
		}
		manager->Wake();
	}

	void RequestManager::UnregisterRequestImpl(Request &request)
	{
		auto manager = static_cast<RequestManagerImpl *>(this);
		{
			std::lock_guard lk(manager->sharedStateMx);
			manager->requestHandlesToUnregister.push_back(request.handle);
		}
		manager->Wake();
	}

	void RequestManagerImpl::HandleWake()
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
		}
	}

	static void HandleWakeThunk()
	{
		auto manager = static_cast<RequestManagerImpl *>(&RequestManager::Ref());
		manager->HandleWake();
	}

	void RequestManagerImpl::Wake()
	{
		emscripten_async_run_in_main_runtime_thread(EM_FUNC_SIG_V, &HandleWakeThunk);
	}

	void RequestManagerImpl::RegisterRequestHandle(std::shared_ptr<RequestHandle> requestHandle)
	{
		auto handle = static_cast<RequestHandleHttp *>(requestHandle.get());
		handle->id = EM_ASM_INT({
			let id = 0;
			while (Module.emscriptenRequestManager.requests[id])
			{
				id += 1;
			}
			let request = {};
			request.fetchResource = UTF8ToString($0);
			request.fetchBody = undefined;
			request.fetchHeaders = new Headers(); 
			Module.emscriptenRequestManager.requests[id] = request;
			return id;
		}, requestHandle->uri.c_str());
		{
			auto userAgentSet = false;
			for (auto &header : handle->headers)
			{
				if (header.name.ToLower() == "user-agent")
				{
					userAgentSet = true;
				}
			}
			if (!userAgentSet)
			{
				handle->headers.push_back({ "user-agent", userAgent });
			}
		}
		for (auto &header : handle->headers)
		{
			EM_ASM({
				Module.emscriptenRequestManager.requests[$0].fetchHeaders.append(
					UTF8ToString($1),
					UTF8ToString($2)
				);
			}, handle->id, header.name.c_str(), header.value.c_str());
		}
		auto &postData = handle->postData;
		if (std::holds_alternative<http::FormData>(postData) && std::get<http::FormData>(postData).size())
		{
			auto &formData = std::get<http::FormData>(postData);
			EM_ASM({
				Module.emscriptenRequestManager.requests[$0].fetchBody = new FormData();
			}, handle->id);
			for (auto &field : formData)
			{
				if (field.filename.has_value())
				{
					EM_ASM({
						Module.emscriptenRequestManager.requests[$0].fetchBody.append(
							UTF8ToString($1),
							new Blob([ HEAP8.slice($2, $2 + $3) ]),
							UTF8ToString($4)
						);
					}, handle->id, field.name.c_str(), &field.value[0], field.value.size(), field.filename->c_str());
				}
				else
				{
					EM_ASM({
						Module.emscriptenRequestManager.requests[$0].fetchBody.append(
							UTF8ToString($1),
							UTF8ToString($2)
						)
					}, handle->id, field.name.c_str(), field.value.c_str());
				}
			}
		}
		else if (std::holds_alternative<http::StringData>(postData) && std::get<http::StringData>(postData).size())
		{
			auto &stringData = std::get<http::StringData>(postData);
			EM_ASM({
				Module.emscriptenRequestManager.requests[$0].fetchBody = new DataView(
					HEAP8.buffer,
					HEAP8.byteOffset + $1,
					$2
				);
			}, handle->id, &stringData[0], stringData.size());
		}
		if (handle->isPost)
		{
			EM_ASM({
				Module.emscriptenRequestManager.requests[$0].fetchMethod = 'POST';
			}, handle->id);
		}
		else
		{
			EM_ASM({
				Module.emscriptenRequestManager.requests[$0].fetchMethod = 'GET';
			}, handle->id);
		}
		if (requestHandle->verb)
		{
			EM_ASM({
				Module.emscriptenRequestManager.requests[$0].fetchMethod = UTF8ToString($1);
			}, handle->id, requestHandle->verb->c_str());
		}
		EM_ASM({
			let request = Module.emscriptenRequestManager.requests[$0];
			let token = $1;
			request.status = 0;
			request.bytesTotal = -1;
			request.bytesDone = 0;
			request.alive = true;
			let updateRequestStatus = () => {
				if (request.alive) {
					Module.emscriptenRequestManager.updateRequestStatusThunk(token);
				}
			};
			request.fetchController = new AbortController();
			fetch(request.fetchResource, {
				method: request.fetchMethod,
				headers: request.fetchHeaders,
				body: request.fetchBody,
				signal: request.fetchController.signal,
			}).then(response => {
				request.statusEarly = response.status;
				let contentLength = response.headers.get('content-length');
				if (contentLength) {
					request.bytesTotal = parseInt(contentLength, 10);
				}
				let reader = response.body.getReader();
				let stream = new ReadableStream({
					start(controller) {
						function read() {
							reader.read().then(({ done, value }) => {
								if (done) {
									return controller.close();
								}
								request.bytesDone += value.byteLength;
								updateRequestStatus();
								controller.enqueue(value);
								read();
							}).catch(err => {
								controller.error(err);
							});
						}
						read();
					}
				});
				request.responseHeaders = [];
				for (let [ name, value ] of response.headers.entries()) {
					request.responseHeaders.push({
						name: name,
						value: value
					});
				}
				return new Response(stream, {
					headers: response.headers
				});
			}).then(output => {
				return output.arrayBuffer();
			}).then(data => {
				request.status = request.statusEarly;
				request.responseData = data;
				updateRequestStatus();
			}).catch(err => {
				console.error(err);
				if (!request.status) {
					request.status = 600;
				}
				updateRequestStatus();
			});
		}, handle->id, handle);
	}

	void RequestManagerImpl::UpdateRequestStatus(RequestHandleHttp *handle)
	{
		assert(handle->id >= 0);
		handle->bytesTotal = EM_ASM_INT({
			return Module.emscriptenRequestManager.requests[$0].bytesTotal;
		}, handle->id);
		handle->bytesDone = EM_ASM_INT({
			return Module.emscriptenRequestManager.requests[$0].bytesDone;
		}, handle->id);
		handle->statusCode = EM_ASM_INT({
			return Module.emscriptenRequestManager.requests[$0].status;
		}, handle->id);
		if (!handle->gotResponse && handle->statusCode)
		{
			auto responseDataSize = EM_ASM_INT({
				let responseData = Module.emscriptenRequestManager.requests[$0].responseData;
				if (responseData) {
					return responseData.byteLength;
				}
				return 0;
			}, handle->id);
			if (responseDataSize)
			{
				handle->responseData.resize(responseDataSize);
				EM_ASM({
					let responseData = Module.emscriptenRequestManager.requests[$0].responseData;
					writeArrayToMemory(new Int8Array(responseData), $1);
				}, handle->id, &handle->responseData[0]);
			}
			auto headerCount = EM_ASM_INT({
				let responseHeaders = Module.emscriptenRequestManager.requests[$0].responseHeaders;
				if (responseHeaders) {
					return responseHeaders.length;
				}
				return 0;
			}, handle->id);
			handle->responseHeaders.resize(headerCount);
			for (auto i = 0; i < headerCount; ++i)
			{
				handle->responseHeaders[i].name = ByteString(std::unique_ptr<char, decltype(&free)>((char *)EM_ASM_PTR({
					return stringToNewUTF8(Module.emscriptenRequestManager.requests[$0].responseHeaders[$1].name);
				}, handle->id, i), free).get());
				handle->responseHeaders[i].value = ByteString(std::unique_ptr<char, decltype(&free)>((char *)EM_ASM_PTR({
					return stringToNewUTF8(Module.emscriptenRequestManager.requests[$0].responseHeaders[$1].value);
				}, handle->id, i), free).get());
			}
			handle->gotResponse = true;
			HandleWake();
		}
	}

	void RequestManagerImpl::UnregisterRequestHandle(std::shared_ptr<RequestHandle> requestHandle)
	{
		auto handle = static_cast<RequestHandleHttp *>(requestHandle.get());
		assert(handle->id >= 0);
		EM_ASM({
			let request = Module.emscriptenRequestManager.requests[$0];
			request.alive = false;
			request.fetchController.abort();
			Module.emscriptenRequestManager.requests[$0] = null;
		}, handle->id);
		handle->id = -1;
	}

	RequestManagerPtr RequestManager::Create(ByteString newProxy, ByteString newCafile, ByteString newCapath, bool newDisableNetwork)
	{
		return RequestManagerPtr(new RequestManagerImpl(newProxy, newCafile, newCapath, newDisableNetwork));
	}

	void RequestManagerDeleter::operator ()(RequestManager *ptr) const
	{
		delete static_cast<RequestManagerImpl *>(ptr);
	}
}

void RequestManager_UpdateRequestStatusThunk(http::RequestHandleHttp *handle)
{
	auto manager = static_cast<http::RequestManagerImpl *>(&http::RequestManager::Ref());
	manager->UpdateRequestStatus(handle);
}
