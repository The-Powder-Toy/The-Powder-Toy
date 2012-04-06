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
	if(this->progress!=progress) {
		this->progress = progress;
		if(listener)
			listener->NotifyProgress(this);
	}
}

void Task::notifyStatus(std::string status)
{
	if(this->status!=status) {
		this->status = status;
		if(listener)
			listener->NotifyStatus(this);
	}
}

void Task::notifyDone()
{
	if(listener)
	{
		done = true; listener->NotifyDone(this);
	}
}
