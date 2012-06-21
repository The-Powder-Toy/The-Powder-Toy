/*
 * Task.h
 *
 *  Created on: Apr 6, 2012
 *      Author: Simon
 */

#ifndef TASK_H_
#define TASK_H_

#include <string>
#include <pthread.h>
#include "TaskListener.h"

class TaskListener;
class Task {
public:
	void SetTaskListener(TaskListener * listener);
	void Start();
	int GetProgress();
	bool GetDone();
	std::string GetStatus();
	void Poll();
	Task() {}
	virtual ~Task();
protected:
	int progress;
	bool done;
	std::string status;

	int thProgress;
	bool thDone;
	std::string thStatus;

	TaskListener * listener;
	pthread_t doWorkThread;
	pthread_mutex_t taskMutex;
	pthread_cond_t taskCond;


	virtual void before();
	virtual void after();
	virtual void doWork();
	static void * doWork_helper(void * ref);

	void notifyProgress(int progress);
	void notifyStatus(std::string status);
	void notifyDone();
};

#endif /* TASK_H_ */
