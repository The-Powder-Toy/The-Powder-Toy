#ifndef TASK_H_
#define TASK_H_

#include "common/String.h"
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
	String GetError();
	String GetStatus();
	void Poll();
	Task() : listener(NULL) { progress = 0; thProgress = 0; }
	virtual ~Task();
protected:
	int progress;
	bool done;
	bool success;
	String status;
	String error;

	int thProgress;
	bool thDone;
	bool thSuccess;
	String thStatus;
	String thError;

	TaskListener * listener;
	pthread_t doWorkThread;
	pthread_mutex_t taskMutex;
	pthread_cond_t taskCond;


	virtual void before();
	virtual void after();
	virtual bool doWork();
	TH_ENTRY_POINT static void * doWork_helper(void * ref);

	virtual void notifyProgress(int progress);
	virtual void notifyError(String error);
	virtual void notifyStatus(String status);

	virtual void notifyProgressMain();
	virtual void notifyErrorMain();
	virtual void notifyStatusMain();
	virtual void notifyDoneMain();
};

#endif /* TASK_H_ */
