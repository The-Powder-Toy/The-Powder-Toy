#pragma once

#include "Panel.h"

namespace ui
{
	class ScrollPanel: public Panel
	{
	protected:
		int scrollBarWidth;
		Point maxOffset;
		float offsetX;
		float offsetY;
		float yScrollVel;
		float xScrollVel;
		bool isMouseInsideScrollbar;
		bool isMouseInsideScrollbarArea;
		bool scrollbarSelected;
		int scrollbarInitialYOffset;
		int scrollbarInitialYClick;
		int scrollbarClickLocation;
	public:
		ScrollPanel(Point position, Point size);

		int GetScrollLimit();
		void SetScrollPosition(int position);

		virtual void Draw(const Point& screenPos);
		virtual void XTick(float dt);
		virtual void XOnMouseWheelInside(int localx, int localy, int d);
		virtual void XOnMouseClick(int localx, int localy, unsigned int button);
		virtual void XOnMouseUp(int x, int y, unsigned int button);
		virtual void XOnMouseMoved(int localx, int localy, int dx, int dy);
	};
}
