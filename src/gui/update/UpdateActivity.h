#pragma once

class Task;
class TaskWindow;
class UpdateActivity
{
	Task * updateDownloadTask;
	TaskWindow * updateWindow;
public:
	UpdateActivity();
	virtual ~UpdateActivity();
	void Exit();
	virtual void NotifyDone(Task * sender);
	virtual void NotifyError(Task * sender);
};
