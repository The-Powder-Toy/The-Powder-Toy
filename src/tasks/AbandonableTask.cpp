#include "AbandonableTask.h"

void AbandonableTask::doWork_wrapper()
{
	Task::doWork_wrapper();
	done_cv.notify_one();

	bool abandoned;
	{
		std::lock_guard<std::mutex> g(taskMutex);
		abandoned = thAbandoned;
	}
	if (abandoned)
	{
		delete this;
	}
}

void AbandonableTask::Finish()
{
	{
		std::unique_lock<std::mutex> l(taskMutex);
		done_cv.wait(l, [this]() { return thDone; });
	}

	// Poll to make sure that the rest of the Task knows that it's
	// done, not just us. This has to be done because the thread that started
	// the AbandonableTask may or may not call Poll before calling Finish.
	// This may call callbacks.
	Poll();

	delete this;
}

void AbandonableTask::Abandon()
{
	bool delete_this = false;
	{
		std::lock_guard<std::mutex> g(taskMutex);
		if (thDone)
		{
			// If thDone is true, the thread has already finished. We're
			// not calling Poll because it may call callbacks, which
			// an abandoned task shouldn't do. Instead we just delete the
			// AbandonableTask after unlocking the mutex.
			delete_this = true;
		}
		else
		{
			// If at this point thDone is still false, the thread is still
			// running, meaning we can safely set thAbandoned and let
			// AbandonableTask::doWork_wrapper delete the AbandonableTask later.
			thAbandoned = true;
		}
	}

	if (delete_this)
	{
		delete this;
	}
}

AbandonableTask::AbandonableTask() :
	thAbandoned(false)
{
}

AbandonableTask::~AbandonableTask()
{
}

