/*
 * TaskWindow.h
 *
 *  Created on: Apr 6, 2012
 *      Author: Simon
 */

#ifndef TASKWINDOW_H_
#define TASKWINDOW_H_

#include <string>
#include "interface/Label.h"
#include "interface/Window.h"
#include "tasks/TaskListener.h"

class Task;
class TaskWindow: public ui::Window, public TaskListener {
	Task * task;
	std::string title;
	int progress;
	bool done;
	ui::Label * statusLabel;
public:
	TaskWindow(std::string title_, Task * task_);
	virtual void NotifyStatus(Task * task);
	virtual void NotifyDone(Task * task);
	virtual void NotifyProgress(Task * task);
	virtual void OnDraw();
	virtual ~TaskWindow();
};

#endif /* TASKWINDOW_H_ */
