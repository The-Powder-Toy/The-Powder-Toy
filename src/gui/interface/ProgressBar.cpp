#include "ProgressBar.h"

#include "graphics/Graphics.h"

#include "gui/Style.h"

#include <algorithm>

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

void ProgressBar::Draw(const Point &screenPos)
{
	Graphics *g = GetGraphics();
	g->DrawRect(RectSized(screenPos, Size), 0xFFFFFF_rgb);
	auto inner = RectSized(screenPos + Vec2{ 2, 2 }, Size - Vec2{ 4, 4 });
	auto drawContent = [this, screenPos, g, inner](int beginX, int endX, ui::Colour bgColour, ui::Colour textColour) {
		auto clip = RectSized(inner.TopLeft + Vec2{ beginX, 0 }, Vec2{ endX - beginX, inner.Size().Y }) & g->GetClipRect();
		g->SwapClipRect(clip);
		if (bgColour.Alpha)
		{
			g->DrawFilledRect(inner, bgColour.NoAlpha());
		}
		g->BlendText(screenPos + Vec2{
			(Size.X - (Graphics::TextSize(progressStatus).X - 1)) / 2,
			(Size.Y - 8) / 2
		}, progressStatus, textColour);
		g->SwapClipRect(clip);
	};
	drawContent(0, inner.Size().X, 0x000000_rgb .WithAlpha(0), 0xFFFFFF_rgb .WithAlpha(255));
	if (progress == -1)
	{
		constexpr auto size = 40;
		auto pos = int(inner.Size().X * intermediatePos / 100);
		drawContent(pos, pos + size, style::Colour::WarningTitle, 0x000000_rgb .WithAlpha(255));
		pos -= inner.Size().X;
		drawContent(pos, pos + size, style::Colour::WarningTitle, 0x000000_rgb .WithAlpha(255));
	}
	else
	{
		drawContent(0, inner.Size().X * progress / 100, style::Colour::WarningTitle, 0x000000_rgb .WithAlpha(255));
	}
}

void ProgressBar::Tick(float dt)
{
	intermediatePos += 1.0f*dt;
	if(intermediatePos>100.0f)
		intermediatePos = 0.0f;
}
