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

		void Draw(const Point& screenPos) override;
		void XTick(float dt) override;
		void XOnMouseWheelInside(int localx, int localy, int d) override;
		void XOnMouseClick(int localx, int localy, unsigned int button) override;
		void XOnMouseUp(int x, int y, unsigned int button) override;
		void XOnMouseMoved(int localx, int localy, int dx, int dy) override;
	};
}
