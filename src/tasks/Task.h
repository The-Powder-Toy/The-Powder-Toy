#ifndef TASK_H_
#define TASK_H_

#include <string>
#include "common/tpt-thread.h"
#include "TaskListener.h"
#include "Config.h"

class TaskListener;
class Task {
public:
	void AddTaskListener(TaskListener * listener);
	void Start();
	int GetProgress();
	bool GetDone();
	bool GetSuccess();
	std::string GetError();
	std::string GetStatus();
	void Poll();
	Task() : listener(NULL) { progress = 0; thProgress = 0; }
	virtual ~Task();
protected:
	int progress;
	bool done;
	bool success;
	std::string status;
	std::string error;

	int thProgress;
	bool thDone;
	bool thSuccess;
	std::string thStatus;
	std::string thError;

	TaskListener * listener;
	pthread_t doWorkThread;
	pthread_mutex_t taskMutex;
	pthread_cond_t taskCond;


	virtual void before();
	virtual void after();
	virtual bool doWork();
	TH_ENTRY_POINT static void * doWork_helper(void * ref);

	virtual void notifyProgress(int progress);
	virtual void notifyError(std::string error);
	virtual void notifyStatus(std::string status);

	virtual void notifyProgressMain();
	virtual void notifyErrorMain();
	virtual void notifyStatusMain();
	virtual void notifyDoneMain();
};

#endif /* TASK_H_ */
