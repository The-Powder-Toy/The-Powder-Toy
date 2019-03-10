#ifndef TASKWINDOW_H_
#define TASKWINDOW_H_

#include "gui/interface/Label.h"
#include "gui/interface/Window.h"
#include "tasks/TaskListener.h"

class Task;
class TaskWindow: public ui::Window, public TaskListener {
	Task * task;
	String title;
	int progress;
	float intermediatePos;
	bool done;
	bool closeOnDone;
	ui::Label * statusLabel;
	String progressStatus;
public:
	TaskWindow(String title_, Task * task_, bool closeOnDone = true);
	void NotifyStatus(Task * task) override;
	void NotifyDone(Task * task) override;
	void NotifyProgress(Task * task) override;
	void NotifyError(Task * task) override;
	void OnTick(float dt) override;
	void OnDraw() override;
	void Exit();
	virtual ~TaskWindow();
};

#endif /* TASKWINDOW_H_ */
