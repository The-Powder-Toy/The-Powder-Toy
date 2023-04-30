#include "ProgressBar.h"

#include "graphics/Graphics.h"

#include "gui/Style.h"

#include "common/tpt-minmax.h"

using namespace ui;

ProgressBar::ProgressBar(Point position, Point size, int startProgress, String startStatus):
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

void ProgressBar::SetStatus(String status)
{
	progressStatus = status;
}

String ProgressBar::GetStatus()
{
	return progressStatus;
}

void ProgressBar::Draw(const Point & screenPos)
{
	Graphics * g = GetGraphics();

	ui::Colour progressBarColour = style::Colour::WarningTitle;

	g->DrawRect(RectSized(Position, Size), 0xFFFFFF_rgb);

	if(progress!=-1)
	{
		if(progress > 0)
		{
			if(progress > 100)
				progress = 100;
			float size = float(Size.X-4)*(float(progress)/100.0f); // TIL...
			size = std::min(std::max(size, 0.0f), float(Size.X-4));
			g->DrawFilledRect(RectSized(screenPos + Vec2{ 2, 2 }, Vec2{ int(size), Size.Y-4 }), progressBarColour.NoAlpha());
		}
	} else {
		int size = 40, rsize = 0;
		float position = float(Size.X-4)*(intermediatePos/100.0f);
		if(position + size - 1 > Size.X-4)
		{
			size = int((Size.X-4)-position+1);
			rsize = 40-size;
		}
		g->DrawFilledRect(RectSized(screenPos + Vec2{ 2 + int(position), 2 }, Vec2{ size, Size.Y-4 }), progressBarColour.NoAlpha());
		if(rsize)
		{
			g->DrawFilledRect(RectSized(screenPos + Vec2{ 2, 2 }, Vec2{ rsize, Size.Y-4 }), progressBarColour.NoAlpha());
		}
	}
	g->BlendText(screenPos + Vec2{ ((Size.X-(Graphics::TextSize(progressStatus).X - 1))/2), (Size.Y-8)/2 }, progressStatus, progress<50 ? 0xFFFFFF_rgb .WithAlpha(255) : 0x000000_rgb .WithAlpha(255));
}

void ProgressBar::Tick(float dt)
{
	intermediatePos += 1.0f*dt;
	if(intermediatePos>100.0f)
		intermediatePos = 0.0f;
}
