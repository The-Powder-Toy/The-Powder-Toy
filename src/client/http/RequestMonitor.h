#ifndef REQUESTMONITOR_H
#define REQUESTMONITOR_H

#include <type_traits>
#include <cassert>

namespace http
{
	template<class R>
	class RequestMonitor
	{
		R *request;

	protected:
		RequestMonitor() :
			request(nullptr)
		{
		}

		virtual ~RequestMonitor()
		{
			if (request)
			{
				request->Cancel();
			}
		}

		void RequestPoll()
		{
			if (request && request->CheckDone())
			{
				OnResponse(request->Finish());
				request = nullptr;
			}
		}

		template<class... Args>
		void RequestSetup(Args&&... args)
		{
			assert(!request);
			request = new R(std::forward<Args>(args)...);
		}

		void RequestStart()
		{
			assert(request);
			request->Start();
		}

		virtual void OnResponse(typename std::result_of<decltype(&R::Finish)(R)>::type v) = 0;
	};
}

#endif // REQUESTMONITOR_H


