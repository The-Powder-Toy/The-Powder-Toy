#include "ProgressBar.h"
#include "common/tpt-minmax.h"
#include "graphics/Graphics.h"
#include "gui/Style.h"

using namespace ui;

ProgressBar::ProgressBar(Point position, Point size, int startProgress, std::string startStatus):
	Component(position, size),
	progress(0),
	intermediatePos(0.0f),
	progressStatus("")
{
	SetStatus(startStatus);
	SetProgress(startProgress);
}

void ProgressBar::SetProgress(int progress)
{
	this->progress = progress;
	if(this->progress > 100)
		this->progress = 100;
}

int ProgressBar::GetProgress()
{
	return progress;
}

void ProgressBar::SetStatus(std::string status)
{
	progressStatus = status;
}

std::string ProgressBar::GetStatus()
{
	return progressStatus;
}

void ProgressBar::Draw(const Point & screenPos)
{
	Graphics * g = GetGraphics();

	ui::Colour progressBarColour = style::Colour::WarningTitle;

	g->drawrect(screenPos.X, screenPos.Y, Size.X, Size.Y, 255, 255, 255, 255);

	if(progress!=-1)
	{
		if(progress > 0)
		{
			if(progress > 100)
				progress = 100;
			float size = float(Size.X-4)*(float(progress)/100.0f); // TIL...
			size = std::min(std::max(size, 0.0f), float(Size.X-4));
			g->fillrect(screenPos.X + 2, screenPos.Y + 2, size, Size.Y-4, progressBarColour.Red, progressBarColour.Green, progressBarColour.Blue, 255);
		}
	} else {
		int size = 40, rsize = 0;
		float position = float(Size.X-4)*(intermediatePos/100.0f);
		if(position + size - 1 > Size.X-4)
		{
			size = (Size.X-4)-position+1;
			rsize = 40-size;
		}
		g->fillrect(screenPos.X + 2 + position, screenPos.Y + 2, size, Size.Y-4, progressBarColour.Red, progressBarColour.Green, progressBarColour.Blue, 255);
		if(rsize)
		{
			g->fillrect(screenPos.X + 2, screenPos.Y + 2, rsize, Size.Y-4, progressBarColour.Red, progressBarColour.Green, progressBarColour.Blue, 255);
		}
	}
	if(progress<50)
		g->drawtext(screenPos.X + ((Size.X-Graphics::textwidth(progressStatus.c_str()))/2), screenPos.Y + (Size.Y-8)/2, progressStatus, 255, 255, 255, 255);
	else
		g->drawtext(screenPos.X + ((Size.X-Graphics::textwidth(progressStatus.c_str()))/2), screenPos.Y + (Size.Y-8)/2, progressStatus, 0, 0, 0, 255);
}

void ProgressBar::Tick(float dt)
{
	intermediatePos += 1.0f*dt;
	if(intermediatePos>100.0f)
		intermediatePos = 0.0f;
}
