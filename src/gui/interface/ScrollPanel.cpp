#include "ScrollPanel.h"
#include "Engine.h"
#include "graphics/Graphics.h"
#include "Misc.h"
#include "PowderToySDL.h"
#include "Window.h"
#include <algorithm>

using namespace ui;

ScrollPanel::ScrollPanel(Point position, Point size):
	Panel(position, size),
	scrollBarWidth(0),
	maxOffset(0, 0),
	offsetX(0),
	offsetY(0),
	isMouseInsideScrollbar(false),
	isMouseInsideScrollbarArea(false),
	scrollbarSelected(false),
	scrollbarInitialYOffset(0),
	scrollbarInitialYClick(0),
	scrollbarClickLocation(0),
	scrollLastTick(int64_t(Engine::Ref().LastTick()))
{
}

int ScrollPanel::GetScrollLimit()
{
	if (ViewportPosition.Y == 0)
		return -1;
	else if (maxOffset.Y == -ViewportPosition.Y)
		return 1;
	return 0;
}

void ScrollPanel::SetScrollPosition(int position)
{
	offsetY = float(position);
	ViewportPosition.Y = -position;
}

void ScrollPanel::XOnMouseWheelInside(int localx, int localy, int d)
{
	if (!d)
		return;
	if (ui::Engine::Ref().MomentumScroll)
		yScrollVel.SetValue(yScrollVel.GetValue() - d * 2);
	else
		yScrollVel.SetValue(yScrollVel.GetValue() - d * 20);
}

void ScrollPanel::Draw(const Point& screenPos)
{
	Panel::Draw(screenPos);

	Graphics * g = GetGraphics();

	//Vertical scroll bar
	if (maxOffset.Y>0 && InnerSize.Y>0)
	{
		float scrollHeight = float(Size.Y)*(float(Size.Y)/float(InnerSize.Y));
		float scrollPos = 0;
		if (-ViewportPosition.Y>0)
		{
			scrollPos = float(Size.Y-scrollHeight)*(float(offsetY)/float(maxOffset.Y));
		}

		g->BlendFilledRect(RectSized(screenPos + Vec2{ Size.X - scrollBarWidth, 0 }, { scrollBarWidth, Size.Y }), 0x7D7D7D_rgb .WithAlpha(100));
		g->DrawFilledRect(RectSized(screenPos + Vec2{ Size.X - scrollBarWidth, int(scrollPos) }, { scrollBarWidth, int(scrollHeight)+1 }), 0xFFFFFF_rgb);
	}
}

void ScrollPanel::XOnMouseDown(int x, int y, unsigned int button)
{
	if (MouseDownInside)
	{
		CancelPanning();
		if (isMouseInsideScrollbar)
		{
			scrollbarSelected = true;
			scrollbarInitialYOffset = int(offsetY);
		}
		initialOffsetY = int(offsetY);
		scrollbarInitialYClick = y - Position.Y;
		scrollbarClickLocation = 100;
	}
}

void ScrollPanel::CancelPanning()
{
	panning = false;
	panHistory = {};
	yScrollVel = 0;
}

void ScrollPanel::XOnMouseUp(int x, int y, unsigned int button)
{
	scrollbarSelected = false;
	auto oldPanHistory = panHistory;
	CancelPanning();
	{
		auto it = oldPanHistory.end();
		while (it != oldPanHistory.begin() && *(it - 1))
		{
			--it;
		}
		if (it < oldPanHistory.end())
		{
			auto offsetYDiff = oldPanHistory.back()->offsetY - (*it)->offsetY;
			auto tickDiff = oldPanHistory.back()->ticks - (*it)->ticks;
			yScrollVel.SetValue(yScrollVel.GetValue() + offsetYDiff / tickDiff * (1000.f / Engine::Ref().GetFps()));
		}
	}
	isMouseInsideScrollbarArea = false;
	scrollbarClickLocation = 0;
}

void ScrollPanel::XOnMouseMoved(int x, int y)
{
	if(maxOffset.Y>0 && InnerSize.Y>0)
	{
		auto oldViewportPositionY = ViewportPosition.Y;
		float scrollHeight = float(Size.Y)*(float(Size.Y)/float(InnerSize.Y));
		float scrollPos = 0;
		if (-ViewportPosition.Y>0)
		{
			scrollPos = float(Size.Y-scrollHeight)*(float(offsetY)/float(maxOffset.Y));
		}

		if (scrollbarSelected)
		{
			if (x > 0)
			{
				auto scrollY = int(float(y-scrollbarInitialYClick)/float(Size.Y)*float(InnerSize.Y)+scrollbarInitialYOffset);
				ViewportPosition.Y = -scrollY;
				offsetY = float(scrollY);
			}
			else
			{
				ViewportPosition.Y = -scrollbarInitialYOffset;
				offsetY = float(scrollbarInitialYOffset);
			}
		}
		else if (MouseDownInside)
		{
			Vec2<int> mouseAt{ x, y };
			if (Engine::Ref().TouchUI && iabs(scrollbarInitialYClick - mouseAt.Y) > PanOffsetThreshold)
			{
				panning = true;
				for (auto *child : children)
				{
					child->MouseDownInside = false;
				}
				GetParentWindow()->FocusComponent(nullptr);
			}
		}

		if (x > (Size.X-scrollBarWidth) && x < Size.X)
		{
			isMouseInsideScrollbar = y > scrollPos && y < scrollPos+scrollHeight;
			isMouseInsideScrollbarArea = true;
		}
		else
			isMouseInsideScrollbar = isMouseInsideScrollbarArea = false;

		if (oldViewportPositionY != ViewportPosition.Y)
		{
			PropagateMouseMove();
		}
	}
}

void ScrollPanel::XTick()
{
	auto oldViewportPositionY = ViewportPosition.Y;

	if (panning)
	{
		auto scrollY = initialOffsetY + scrollbarInitialYClick - (Engine::Ref().GetMouseY() - GetScreenPos().Y);
		ViewportPosition.Y = -scrollY;
		offsetY = float(scrollY);
		PanPoint p{ offsetY, GetTicks() };
		if (!(panHistory.back() && panHistory.back()->ticks == p.ticks))
		{
			std::copy(panHistory.begin() + 1, panHistory.end(), panHistory.begin());
			panHistory.back() = p;
		}
	}

	if (xScrollVel.GetValue() > 7.0f) xScrollVel.SetValue(7.0f);
	if (xScrollVel.GetValue() < -7.0f) xScrollVel.SetValue(-7.0f);
	if (xScrollVel.GetValue() > -0.5f && xScrollVel.GetValue() < 0.5f)
		xScrollVel.SetValue(0);

	maxOffset = InnerSize-Size;
	maxOffset.Y = std::max(0, maxOffset.Y);
	maxOffset.X = std::max(0, maxOffset.X);

	auto oldOffsetY = int(offsetY);
	{
		// the correct way to do this would be to add another profile to ui::Fade that analytically
		// integrates the value of the fade and assign that here; way too much hassle for now though
		auto now = int64_t(Engine::Ref().LastTick());
		auto diff = now - scrollLastTick;
		scrollLastTick = now;
		offsetY += yScrollVel * diff * 0.06f;
		offsetX += xScrollVel * diff * 0.06f;
	}


	if (!ui::Engine::Ref().MomentumScroll)
	{
		yScrollVel.SetValue(0.0f);
		xScrollVel.SetValue(0.0f);
	}


	if (oldOffsetY!=int(offsetY))
	{
		if (offsetY<0)
		{
			offsetY = 0;
			yScrollVel = 0;
		}
		else if (offsetY>maxOffset.Y)
		{
			offsetY = float(maxOffset.Y);
			yScrollVel = 0;
		}
		ViewportPosition.Y = -int(offsetY);
	}
	else
	{
		if (offsetY<0)
		{
			offsetY = 0;
			yScrollVel = 0;
			ViewportPosition.Y = -int(offsetY);
		}
		else if (offsetY>maxOffset.Y)
		{
			offsetY = float(maxOffset.Y);
			ViewportPosition.Y = -int(offsetY);
		}
	}

	if (MouseInside && scrollBarWidth < 6)
		scrollBarWidth++;
	else if (!MouseInside && scrollBarWidth > 0 && !scrollbarSelected)
		scrollBarWidth--;

	if (isMouseInsideScrollbarArea && scrollbarClickLocation && !scrollbarSelected)
	{
		float scrollHeight = float(Size.Y)*(float(Size.Y)/float(InnerSize.Y));
		float scrollPos = 0;
		if (-ViewportPosition.Y > 0)
			scrollPos = float(Size.Y-scrollHeight)*(float(offsetY)/float(maxOffset.Y));

		if (scrollbarInitialYClick <= scrollPos)
			scrollbarClickLocation = -1;
		else if (scrollbarInitialYClick >= scrollPos+scrollHeight)
			scrollbarClickLocation = 1;
		else
			scrollbarClickLocation = 0;

		offsetY += scrollbarClickLocation*scrollHeight/10;
		ViewportPosition.Y -= int(scrollbarClickLocation*scrollHeight/10);
	}

	if (oldViewportPositionY != ViewportPosition.Y)
	{
		PropagateMouseMove();
	}
}
