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
	void NotifyStatus(Task * task) override;
	void NotifyDone(Task * task) override;
	void NotifyProgress(Task * task) override;
	void NotifyError(Task * task) override;
	void OnTick(float dt) override;
	void OnDraw() override;
	virtual void Exit();
	~TaskWindow() override;
};

#endif /* TASKWINDOW_H_ */
