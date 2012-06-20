/*
 * TaskWindow.cpp
 *
 *  Created on: Apr 6, 2012
 *      Author: Simon
 */

#include "interface/Label.h"
#include "TaskWindow.h"
#include "Task.h"

TaskWindow::TaskWindow(std::string title_, Task * task_):
	task(task_),
	title(title_),
	ui::Window(ui::Point(-1, -1), ui::Point(300, 200)),
	progress(0),
	done(false)
{

	ui::Label * tempLabel = new ui::Label(ui::Point(3, 3), ui::Point(Size.X-6, 16), title);
	AddComponent(tempLabel);

	statusLabel = new ui::Label(ui::Point(3, 19), ui::Point(Size.X-6, 16), "");
	AddComponent(statusLabel);

	ui::Engine::Ref().ShowWindow(this);

	task->SetTaskListener(this);
	task->Start();
}

void TaskWindow::NotifyStatus(Task * task)
{
	statusLabel->SetText(task->GetStatus());
}

void TaskWindow::NotifyDone(Task * task)
{
	if(ui::Engine::Ref().GetWindow()==this)
	{
		ui::Engine::Ref().CloseWindow();
		delete this;
	}
}

void TaskWindow::NotifyProgress(Task * task)
{
	progress = task->GetProgress();
}

void TaskWindow::OnTick(float dt)
{
	task->Poll();
}

void TaskWindow::OnDraw()
{
	Graphics * g = ui::Engine::Ref().g;
	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 255, 255, 255, 255);

	g->drawrect(Position.X + 20, Position.Y + 36, Size.X-40, 24, 255, 255, 255, 255);

	float size = float(Size.X-40)*(float(progress)/100.0f); // TIL...
	g->fillrect(Position.X + 20, Position.Y + 36, size, 24, 255, 255, 255, 255);
}

TaskWindow::~TaskWindow() {
	delete task;
}

