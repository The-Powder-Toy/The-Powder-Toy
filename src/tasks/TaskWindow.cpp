#include <sstream>
#include "common/tpt-minmax.h"
#include "gui/interface/Label.h"
#include "gui/interface/Engine.h"
#include "TaskWindow.h"
#include "gui/dialogues/ErrorMessage.h"
#include "gui/Style.h"
#include "Task.h"

TaskWindow::TaskWindow(std::string title_, Task * task_, bool closeOnDone):
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
	std::stringstream pStream;
	if(progress>-1)
	{
		pStream << progress << "%";
	}
	else
	{
		pStream << "Please wait...";
	}
	progressStatus = pStream.str();
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
	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 255, 255, 255, 255);

	g->draw_line(Position.X, Position.Y + Size.Y-17, Position.X + Size.X - 1, Position.Y + Size.Y-17, 255, 255, 255, 255);

	ui::Colour progressBarColour = style::Colour::WarningTitle;

	if(progress!=-1)
	{
		if(progress > 0)
		{
			if(progress > 100)
				progress = 100;
			float size = float(Size.X-4)*(float(progress)/100.0f); // TIL...
			size = std::min(std::max(size, 0.0f), float(Size.X-4));
			g->fillrect(Position.X + 2, Position.Y + Size.Y-15, size, 13, progressBarColour.Red, progressBarColour.Green, progressBarColour.Blue, 255);
		}
	} else {
		int size = 40, rsize = 0;
		float position = float(Size.X-4)*(intermediatePos/100.0f);
		if(position + size - 1 > Size.X-4)
		{
			size = (Size.X-4)-position+1;
			rsize = 40-size;
		}
		g->fillrect(Position.X + 2 + position, Position.Y + Size.Y-15, size, 13, progressBarColour.Red, progressBarColour.Green, progressBarColour.Blue, 255);
		if(rsize)
		{
			g->fillrect(Position.X + 2, Position.Y + Size.Y-15, rsize, 13, progressBarColour.Red, progressBarColour.Green, progressBarColour.Blue, 255);
		}
	}
	if(progress<50)
		g->drawtext(Position.X + ((Size.X-Graphics::textwidth(progressStatus.c_str()))/2), Position.Y + Size.Y-13, progressStatus, 255, 255, 255, 255);
	else
		g->drawtext(Position.X + ((Size.X-Graphics::textwidth(progressStatus.c_str()))/2), Position.Y + Size.Y-13, progressStatus, 0, 0, 0, 255);
}

TaskWindow::~TaskWindow() {
	delete task;
}

