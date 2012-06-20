/*
 * Task.cpp
 *
 *  Created on: Apr 6, 2012
 *      Author: Simon
 */


#include <unistd.h>
#include "Task.h"
#include "TaskListener.h"

void Task::SetTaskListener(TaskListener * listener)
{
	this->listener = listener;
}

void Task::Start()
{
	pthread_mutex_init (&taskMutex, NULL);
	pthread_cond_init(&taskCond, NULL);
	pthread_create(&doWorkThread, 0, &Task::doWork_helper, this);
}

int Task::GetProgress()
{
	return progress;
}

std::string Task::GetStatus()
{
	return status;
}

bool Task::GetDone()
{
	return done;
}

void Task::Poll()
{
	int newProgress;
	bool newDone;
	std::string newStatus;
	pthread_mutex_lock(&taskMutex);
	newProgress = thProgress;
	newDone = thDone;
	newStatus = std::string(thStatus);
	pthread_cond_signal(&taskCond);
	pthread_mutex_unlock(&taskMutex);

	if(newProgress!=progress) {
		progress = newProgress;
		if(listener)
			listener->NotifyProgress(this);
	}
	if(newStatus!=status) {
		status = newStatus;
		if(listener)
			listener->NotifyStatus(this);
	}
	if(newDone!=done)
	{
		done = newDone;
		if(listener)
			listener->NotifyDone(this);
	}

	if(done)
	{
		pthread_join(doWorkThread, NULL);
		pthread_mutex_destroy(&taskMutex);
	}
}

Task::~Task()
{

}

void Task::doWork()
{
	notifyStatus("Fake progress");
	for(int i = 0; i < 100; i++)
	{
		notifyProgress(i);
		usleep((100)*1000);
	}
}

void * Task::doWork_helper(void * ref)
{
	((Task*)ref)->doWork();
	((Task*)ref)->notifyDone();
	return NULL;
}

void Task::notifyProgress(int progress)
{
	pthread_mutex_lock(&taskMutex);
	pthread_cond_wait(&taskCond, &taskMutex);
	thProgress = progress;
	pthread_mutex_unlock(&taskMutex);
}

void Task::notifyStatus(std::string status)
{
	pthread_mutex_lock(&taskMutex);
	pthread_cond_wait(&taskCond, &taskMutex);
	thStatus = status;
	pthread_mutex_unlock(&taskMutex);
}

void Task::notifyDone()
{
	pthread_mutex_lock(&taskMutex);
	pthread_cond_wait(&taskCond, &taskMutex);
	thDone = true;
	pthread_mutex_unlock(&taskMutex);
}
