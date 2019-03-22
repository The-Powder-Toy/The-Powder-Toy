#include "Config.h"
#include "Task.h"
#include "TaskListener.h"

void Task::AddTaskListener(TaskListener * listener)
{
	this->listener = listener;
	notifyProgressMain();
	notifyStatusMain();
}

void Task::Start()
{
	before();
	// This would use a lambda if we didn't use pthreads and if I dared omit
	// the TH_ENTRY_POINT from the function type.
	pthread_create(&doWorkThread, 0, &Task::doWork_helper, this);
	pthread_detach(doWorkThread);
}

int Task::GetProgress()
{
	return progress;
}

String Task::GetStatus()
{
	return status;
}

String Task::GetError()
{
	return error;
}

bool Task::GetDone()
{
	return done;
}

bool Task::GetSuccess()
{
	return success;
}

void Task::Poll()
{
	if(!done)
	{
		int newProgress;
		bool newDone = false;
		bool newSuccess = false;
		String newStatus;
		String newError;
		pthread_mutex_lock(&taskMutex);
		newProgress = thProgress;
		newDone = thDone;
		newSuccess = thSuccess;
		newStatus = thStatus;
		newError = thError;
		pthread_mutex_unlock(&taskMutex);

		success = newSuccess;

		if(newProgress!=progress) {
			progress = newProgress;
			notifyProgressMain();
		}

		if(newError!=error) {
			error = newError;
			notifyErrorMain();
		}

		if(newStatus!=status) {
			status = newStatus;
			notifyStatusMain();
		}

		if(newDone!=done)
		{
			done = newDone;
			after();
			notifyDoneMain();
		}
	}
}

Task::Task() :
	progress(0),
	done(false),
	thProgress(0),
	thDone(false),
	listener(NULL)
{
	pthread_mutex_init(&taskMutex, NULL);
}

Task::~Task()
{
	pthread_mutex_destroy(&taskMutex);
}

void Task::before()
{

}

bool Task::doWork()
{
	notifyStatus("Fake progress");
	for(int i = 0; i < 100; i++)
	{
		notifyProgress(i);
	}
	return true;
}

void Task::after()
{

}

void Task::doWork_wrapper()
{
	bool newSuccess = doWork();
	pthread_mutex_lock(&taskMutex);
	thSuccess = newSuccess;
	thDone = true;
	pthread_mutex_unlock(&taskMutex);
}

TH_ENTRY_POINT void *Task::doWork_helper(void *ref)
{
	((Task *)ref)->doWork_wrapper();
	return NULL;
}

void Task::notifyProgress(int progress)
{
	pthread_mutex_lock(&taskMutex);
	thProgress = progress;
	pthread_mutex_unlock(&taskMutex);
}

void Task::notifyStatus(String status)
{
	pthread_mutex_lock(&taskMutex);
	thStatus = status;
	pthread_mutex_unlock(&taskMutex);
}

void Task::notifyError(String error)
{
	pthread_mutex_lock(&taskMutex);
	thError = error;
	pthread_mutex_unlock(&taskMutex);
}

void Task::notifyProgressMain()
{
	if(listener)
		listener->NotifyProgress(this);
}

void Task::notifyStatusMain()
{
	if(listener)
		listener->NotifyStatus(this);
}

void Task::notifyDoneMain()
{
	if(listener)
		listener->NotifyDone(this);
}

void Task::notifyErrorMain()
{
	if(listener)
		listener->NotifyError(this);
}
