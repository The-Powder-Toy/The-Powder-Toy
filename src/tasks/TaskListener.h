#ifndef TASKLISTENER_H_
#define TASKLISTENER_H_

class Task;
class TaskListener {
public:
	virtual void NotifyDone(Task * task) {}
	virtual void NotifyError(Task * task) {}
	virtual void NotifyProgress(Task * task) {}
	virtual void NotifyStatus(Task * task) {}
	virtual ~TaskListener() {}
};

#endif /* TASK_H_ */
