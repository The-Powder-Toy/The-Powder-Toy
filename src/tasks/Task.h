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
	Task() {}
	virtual ~Task();
protected:
	int progress;
	bool done;
	std::string status;
	TaskListener * listener;
	pthread_t doWorkThread;
	virtual void doWork();
	static void * doWork_helper(void * ref);
	void notifyProgress(int progress);
	void notifyStatus(std::string status);
	void notifyDone();
};

#endif /* TASK_H_ */
