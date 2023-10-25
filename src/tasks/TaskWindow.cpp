#include "TaskWindow.h"

#include "Task.h"

#include "gui/interface/Label.h"
#include "gui/interface/ProgressBar.h"
#include "gui/interface/Engine.h"
#include "gui/dialogues/ErrorMessage.h"
#include "gui/Style.h"

#include "graphics/Graphics.h"

#include <algorithm>

TaskWindow::TaskWindow(String title_, Task * task_, bool closeOnDone):
	ui::Window(ui::Point(-1, -1), ui::Point(240, 60)),
	task(task_),
	title(title_),
	progress(0),
	done(false),
	closeOnDone(closeOnDone),
	progressStatus("0%")
{

	ui::Label * tempLabel = new ui::Label(ui::Point(4, 5), ui::Point(Size.X-8, 15), title);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	tempLabel->SetTextColour(style::Colour::WarningTitle);
	AddComponent(tempLabel);

	statusLabel = new ui::Label(ui::Point(4, 23), ui::Point(Size.X-8, 15), "");
	statusLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	statusLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(statusLabel);

	progressBar = new ui::ProgressBar(Position + Vec2{ 1, Size.Y-16 }, Vec2{ Size.X, 17 });
	AddComponent(progressBar);

	MakeActiveWindow();

	task->AddTaskListener(this);
	task->Start();
}

void TaskWindow::NotifyStatus(Task * task)
{
	statusLabel->SetText(task->GetStatus());
}

void TaskWindow::NotifyError(Task * task)
{
	new ErrorMessage("Error", task->GetError());
	done = true;
}

void TaskWindow::NotifyDone(Task * task)
{
	if(closeOnDone)
		Exit();
	done = true;
}

void TaskWindow::Exit()
{
	if (CloseActiveWindow())
	{
		SelfDestruct();
	}
}

void TaskWindow::NotifyProgress(Task * task)
{
	progress = task->GetProgress();
	if(progress>-1)
		progressStatus = String::Build(progress, "%");
	else
		progressStatus = "Please wait...";
	progressBar->SetProgress(progress);
	progressBar->SetStatus(progressStatus);
}

void TaskWindow::OnTick(float dt)
{
	intermediatePos += 1.0f*dt;
	if(intermediatePos>100.0f)
		intermediatePos = 0.0f;
	task->Poll();
	if (done)
		Exit();
}

void TaskWindow::OnDraw()
{
	Graphics * g = GetGraphics();
	g->DrawFilledRect(RectSized(Position - Vec2{ 1, 1 }, Size + Vec2{ 2, 2 }), 0x000000_rgb);
	g->DrawRect(RectSized(Position, Size), 0xFFFFFF_rgb);
}
