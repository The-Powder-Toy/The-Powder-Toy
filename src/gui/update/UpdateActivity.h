#ifndef UPDATEACTIVITY_H_
#define UPDATEACTIVITY_H_

#include "tasks/TaskWindow.h"

class UpdateActivity {
	Task * updateDownloadTask;
	TaskWindow * updateWindow;
public:
	UpdateActivity();
	virtual ~UpdateActivity();
	void Exit();
	virtual void NotifyDone(Task * sender);
	virtual void NotifyError(Task * sender);
};

#endif /* UPDATEACTIVITY_H_ */
