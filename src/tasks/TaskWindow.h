#ifndef TASKWINDOW_H_
#define TASKWINDOW_H_

#include <string>
#include "gui/interface/Label.h"
#include "gui/interface/Window.h"
#include "tasks/TaskListener.h"

class Task;
class TaskWindow: public ui::Window, public TaskListener {
	Task * task;
	std::string title;
	int progress;
	float intermediatePos;
	bool done;
	bool closeOnDone;
	ui::Label * statusLabel;
	std::string progressStatus;
public:
	TaskWindow(std::string title_, Task * task_, bool closeOnDone = true);
	virtual void NotifyStatus(Task * task);
	virtual void NotifyDone(Task * task);
	virtual void NotifyProgress(Task * task);
	virtual void NotifyError(Task * task);
	virtual void OnTick(float dt);
	virtual void OnDraw();
	virtual void Exit();
	virtual ~TaskWindow();
};

#endif /* TASKWINDOW_H_ */
