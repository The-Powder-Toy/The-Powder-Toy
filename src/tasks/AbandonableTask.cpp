#include "AbandonableTask.h"

#include "Platform.h"

#ifdef DEBUG
# define DEBUGTHREADS
// * Uncomment above if AbandonableTasks cause issues.
// * Three outputs should be possible:
//   * The task is properly finished:
//       AbandonableTask @ [ptr] ctor
//       AbandonableTask @ [ptr] created
//       ...
//       AbandonableTask @ [ptr] finished
//       AbandonableTask @ [ptr] joined
//       AbandonableTask @ [ptr] dtor
//   * The task is abandoned but its thread has finished:
//       AbandonableTask @ [ptr] ctor
//       AbandonableTask @ [ptr] created
//       ...
//       AbandonableTask @ [ptr] abandoned
//       AbandonableTask @ [ptr] joined
//       AbandonableTask @ [ptr] dtor
//   * The task is abandoned before its thread has finished:
//       AbandonableTask @ [ptr] ctor
//       AbandonableTask @ [ptr] created
//       ...
//       AbandonableTask @ [ptr] abandoned
//       ...
//       AbandonableTask @ [ptr] detached
//       AbandonableTask @ [ptr] dtor
// * Anything other than those means something is broken.
#endif

#ifdef DEBUGTHREADS
# include <iostream>
#endif

void AbandonableTask::Start()
{
	thDone = false;
	done = false;
	thAbandoned = false;
	progress = 0;
	status = "";
	before();
	pthread_mutex_init (&taskMutex, NULL);
	pthread_create(&doWorkThread, 0, &AbandonableTask::doWork_helper, this);

#ifdef DEBUGTHREADS
	std::cerr << "AbandonableTask @ " << this << " created" << std::endl;
#endif
}

TH_ENTRY_POINT void * AbandonableTask::doWork_helper(void * ref)
{
	Task::doWork_helper(ref);

	AbandonableTask *task = (AbandonableTask *)ref;
	pthread_mutex_lock(&task->taskMutex);
	pthread_cond_signal(&task->done_cv);
	bool abandoned = task->thAbandoned;
	pthread_mutex_unlock(&task->taskMutex);
	if (abandoned)
	{
		pthread_detach(task->doWorkThread);
		pthread_mutex_destroy(&task->taskMutex);

#ifdef DEBUGTHREADS
		std::cerr << "AbandonableTask @ " << ref << " detached" << std::endl;
#endif

		// We've done Task::~Task's job already.
		task->done = true;

		delete task;
	}
	return NULL;
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
	// done, not just us.
	Poll();

#ifdef DEBUGTHREADS
	std::cerr << "AbandonableTask @ " << this << " finished" << std::endl;
#endif

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
		// AbandonableTask after unlocking the mutex, which will
		// take care of the thread if another Poll hasn't already
		// (i.e. if done is false despite thDone being true).
		delete_this = true;
	}
	else
	{
		// If at this point thDone is still false, the thread is still
		// running, meaning we can safely set thAbandoned and let
		// AbandonableTask::doWork_helper detach the thread
		// and delete the AbandonableTask later.
		thAbandoned = true;
	}
	pthread_mutex_unlock(&taskMutex);

#ifdef DEBUGTHREADS
	std::cerr << "AbandonableTask @ " << this << " abandoned" << std::endl;
#endif

	if (delete_this)
	{
		delete this;
	}
}

void AbandonableTask::Poll()
{
#ifdef DEBUGTHREADS
	bool old_done = done;
#endif
	Task::Poll();
#ifdef DEBUGTHREADS
	if (done != old_done)
	{
		std::cerr << "AbandonableTask @ " << this << " joined" << std::endl;
	}
#endif
}

AbandonableTask::AbandonableTask()
{
	pthread_cond_init(&done_cv, NULL);
#ifdef DEBUGTHREADS
	std::cerr << "AbandonableTask @ " << this << " ctor" << std::endl;
#endif
}

AbandonableTask::~AbandonableTask()
{
#ifdef DEBUGTHREADS
	if (!done)
	{
		// Actually it'll be joined later in Task::~Task, but the debug
		// messages look more consistent this way.
		std::cerr << "AbandonableTask @ " << this << " joined" << std::endl;
	}

	std::cerr << "AbandonableTask @ " << this << " dtor" << std::endl;
#endif
	pthread_cond_destroy(&done_cv);
}

