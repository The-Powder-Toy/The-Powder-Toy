#include "AbandonableTask.h"

#include "Platform.h"

void AbandonableTask::doWork_wrapper()
{
	Task::doWork_wrapper();
	pthread_cond_signal(&done_cv);

	pthread_mutex_lock(&taskMutex);
	bool abandoned = thAbandoned;
	pthread_mutex_unlock(&taskMutex);
	if (abandoned)
	{
		delete this;
	}
}

void AbandonableTask::Finish()
{
	pthread_mutex_lock(&taskMutex);
	while (!thDone)
	{
		pthread_cond_wait(&done_cv, &taskMutex);
	}
	pthread_mutex_unlock(&taskMutex);

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
	pthread_mutex_lock(&taskMutex);
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
	pthread_mutex_unlock(&taskMutex);

	if (delete_this)
	{
		delete this;
	}
}

AbandonableTask::AbandonableTask() :
	thAbandoned(false)
{
	pthread_cond_init(&done_cv, NULL);
}

AbandonableTask::~AbandonableTask()
{
	pthread_cond_destroy(&done_cv);
}

