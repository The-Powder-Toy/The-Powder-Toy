#pragma once
#include "client/StartupInfo.h"

class Task;
class TaskWindow;
class UpdateActivity
{
	Task * updateDownloadTask;
	TaskWindow * updateWindow;
public:
	UpdateActivity(UpdateInfo info);
	virtual ~UpdateActivity();
	void Exit();
	virtual void NotifyDone(Task * sender);
	virtual void NotifyError(Task * sender);
};
