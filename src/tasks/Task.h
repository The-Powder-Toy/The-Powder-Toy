#ifndef TASK_H_
#define TASK_H_
#include "Config.h"

#include "common/String.h"
#include <thread>
#include <mutex>

class TaskListener;
class Task {
public:
	void AddTaskListener(TaskListener * listener);
	virtual void Start();
	int GetProgress();
	bool GetDone();
	bool GetSuccess();
	String GetError();
	String GetStatus();
	virtual void Poll();
	Task();
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

	TaskListener *listener;
	std::mutex taskMutex;

	virtual void before();
	virtual void after();
	virtual bool doWork();
	virtual void doWork_wrapper();

	virtual void notifyProgress(int progress);
	virtual void notifyError(String error);
	virtual void notifyStatus(String status);

	virtual void notifyProgressMain();
	virtual void notifyErrorMain();
	virtual void notifyStatusMain();
	virtual void notifyDoneMain();
};

#endif /* TASK_H_ */
