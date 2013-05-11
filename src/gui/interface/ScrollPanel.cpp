#include <iostream>
#include "ScrollPanel.h"

using namespace ui;

ScrollPanel::ScrollPanel(Point position, Point size):
	Panel(position, size),
	maxOffset(0, 0),
	offsetX(0),
	offsetY(0),
	yScrollVel(0.0f),
	xScrollVel(0.0f),
	scrollBarWidth(0),
	isMouseInsideScrollbar(false),
	scrollbarSelected(false),
	scrollbarInitialYOffset(0),
	scrollbarInitialYClick(0)
{

}

int ScrollPanel::GetScrollLimit()
{
	if(ViewportPosition.Y == 0)
		return -1;
	else if(maxOffset.Y == -ViewportPosition.Y)
		return 1;
	return 0;
}

void ScrollPanel::XOnMouseWheelInside(int localx, int localy, int d)
{
	if(!d)
		return;
	yScrollVel -= d*2;
}

void ScrollPanel::Draw(const Point& screenPos)
{
	Panel::Draw(screenPos);

	Graphics * g = ui::Engine::Ref().g;

	//Vertical scroll bar
	if(maxOffset.Y>0 && InnerSize.Y>0)
	{
		float scrollHeight = float(Size.Y)*(float(Size.Y)/float(InnerSize.Y));
		float scrollPos = 0;
		if(-ViewportPosition.Y>0)
		{
			scrollPos = float(Size.Y-scrollHeight)*(float(offsetY)/float(maxOffset.Y));
		}

		g->fillrect(screenPos.X+(Size.X-scrollBarWidth), screenPos.Y, scrollBarWidth, Size.Y, 125, 125, 125, 100);
		g->fillrect(screenPos.X+(Size.X-scrollBarWidth), screenPos.Y+scrollPos, scrollBarWidth, scrollHeight, 255, 255, 255, 255);
	}
}

void ScrollPanel::XOnMouseClick(int x, int y, unsigned int button)
{
	if (isMouseInsideScrollbar)
	{
		scrollbarSelected = true;
		scrollbarInitialYOffset = offsetY;
		scrollbarInitialYClick = y;
	}
}

void ScrollPanel::XOnMouseUp(int x, int y, unsigned int button)
{
	scrollbarSelected = false;
}

void ScrollPanel::XOnMouseMoved(int x, int y, int dx, int dy)
{
	if(maxOffset.Y>0 && InnerSize.Y>0)
	{
		float scrollHeight = float(Size.Y)*(float(Size.Y)/float(InnerSize.Y));
		float scrollPos = 0;
		if(-ViewportPosition.Y>0)
		{
			scrollPos = float(Size.Y-scrollHeight)*(float(offsetY)/float(maxOffset.Y));
		}
		
		if (scrollbarSelected)
		{
			if (x > 0)
			{
				int scrollY = float(y-scrollbarInitialYClick)/float(Size.Y)*float(InnerSize.Y)+scrollbarInitialYOffset;
				ViewportPosition.Y = -scrollY;
				offsetY = scrollY;
			}
			else
			{
				ViewportPosition.Y = -scrollbarInitialYOffset;
				offsetY = scrollbarInitialYOffset;
			}
		}

		if (x > (Size.X-scrollBarWidth) && x < (Size.X-scrollBarWidth)+scrollBarWidth && y > scrollPos && y < scrollPos+scrollHeight)
			isMouseInsideScrollbar = true;
		else
			isMouseInsideScrollbar = false;
	}
}

void ScrollPanel::XTick(float dt)
{
	//if(yScrollVel > 7.0f) yScrollVel = 7.0f;
	//if(yScrollVel < -7.0f) yScrollVel = -7.0f;
	if(yScrollVel > -0.5f && yScrollVel < 0.5)
		yScrollVel = 0;

	if(xScrollVel > 7.0f) xScrollVel = 7.0f;
	if(xScrollVel < -7.0f) xScrollVel = -7.0f;
	if(xScrollVel > -0.5f && xScrollVel < 0.5)
		xScrollVel = 0;

	maxOffset = InnerSize-Size;
	maxOffset.Y = std::max(0, maxOffset.Y);
	maxOffset.X = std::max(0, maxOffset.X);

	int oldOffsetY = offsetY;
	offsetY += yScrollVel;
	int oldOffsetX = offsetX;
	offsetX += xScrollVel;

	yScrollVel*=0.98f;
	xScrollVel*=0.98f;

	if(oldOffsetY!=int(offsetY))
	{
		if(offsetY<0)
		{
			offsetY = 0;
			yScrollVel = 0;
			//commentsBegin = true;
			//commentsEnd = false;
		}
		else if(offsetY>maxOffset.Y)
		{
			offsetY = maxOffset.Y;
			yScrollVel = 0;
			//commentsEnd = true;
			//commentsBegin = false;
		}
		else
		{
			//commentsEnd = false;
			//commentsBegin = false;
		}
		ViewportPosition.Y = -offsetY;
	}
	else
	{
		if(offsetY<0)
		{
			offsetY = 0;
			yScrollVel = 0;
			ViewportPosition.Y = -offsetY;
		}
		else if(offsetY>maxOffset.Y)
		{
			offsetY = maxOffset.Y;
			ViewportPosition.Y = -offsetY;
		}
	}

	if(mouseInside && scrollBarWidth < 6)
		scrollBarWidth++;
	else if(!mouseInside && scrollBarWidth > 0 && !scrollbarSelected)
		scrollBarWidth--;
}
