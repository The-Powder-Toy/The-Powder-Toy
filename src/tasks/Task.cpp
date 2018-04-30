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
	thDone = false;
	done = false;
	progress = 0;
	status = "";
	//taskMutex = PTHREAD_MUTEX_INITIALIZER;
	before();
	pthread_mutex_init (&taskMutex, NULL);
	pthread_create(&doWorkThread, 0, &Task::doWork_helper, this);
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

			pthread_join(doWorkThread, NULL);
			pthread_mutex_destroy(&taskMutex);

			after();

			notifyDoneMain();
		}
	}
}

Task::~Task()
{
	if(!done)
	{
		pthread_join(doWorkThread, NULL);
		pthread_mutex_destroy(&taskMutex);
	}
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

TH_ENTRY_POINT void * Task::doWork_helper(void * ref)
{
	bool newSuccess = ((Task*)ref)->doWork();
	pthread_mutex_lock(&((Task*)ref)->taskMutex);
	((Task*)ref)->thSuccess = newSuccess;
	((Task*)ref)->thDone = true;
	pthread_mutex_unlock(&((Task*)ref)->taskMutex);
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
