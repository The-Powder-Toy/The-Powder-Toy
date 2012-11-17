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
	public:
		ScrollPanel(Point position, Point size);

		int GetScrollLimit();

		virtual void Draw(const Point& screenPos);
		virtual void XTick(float dt);
		virtual void XOnMouseWheelInside(int localx, int localy, int d);
	};
}