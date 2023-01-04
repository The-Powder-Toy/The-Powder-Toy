#pragma once

class Task;
class TaskListener {
public:
	virtual void NotifyDone(Task * task) {}
	virtual void NotifyError(Task * task) {}
	virtual void NotifyProgress(Task * task) {}
	virtual void NotifyStatus(Task * task) {}
	virtual ~TaskListener() {}
};
