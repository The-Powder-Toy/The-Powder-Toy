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
	std::thread([this]() { doWork_wrapper(); }).detach();
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
		{
			std::lock_guard<std::mutex> g(taskMutex);
			newProgress = thProgress;
			newDone = thDone;
			newSuccess = thSuccess;
			newStatus = thStatus;
			newError = thError;
		}

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
}

Task::~Task()
{
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
	{
		std::lock_guard<std::mutex> g(taskMutex);
		thSuccess = newSuccess;
		thDone = true;
	}
}

void Task::notifyProgress(int progress)
{
	std::lock_guard<std::mutex> g(taskMutex);
	thProgress = progress;
}

void Task::notifyStatus(String status)
{
	std::lock_guard<std::mutex> g(taskMutex);
	thStatus = status;
}

void Task::notifyError(String error)
{
	std::lock_guard<std::mutex> g(taskMutex);
	thError = error;
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
