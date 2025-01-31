#pragma once
#include "Panel.h"
#include "Fade.h"
#include <optional>
#include <array>

namespace ui
{
	class ScrollPanel: public Panel
	{
		void CancelPanning();

	protected:
		int scrollBarWidth;
		Point maxOffset;
		float offsetX;
		float offsetY;
		ui::Fade yScrollVel{ ui::Fade::ExponentialProfile{ 0.297553f, 0.5f }, 0, 0 };
		ui::Fade xScrollVel{ ui::Fade::ExponentialProfile{ 0.297553f, 0.5f }, 0, 0 };
		bool isMouseInsideScrollbar;
		bool isMouseInsideScrollbarArea;
		bool scrollbarSelected;
		int scrollbarInitialYOffset;
		int scrollbarInitialYClick;
		int scrollbarClickLocation;
		int64_t scrollLastTick;
		int initialOffsetY;
		bool panning = false;
		static constexpr int PanOffsetThreshold = 10;
		static constexpr int PanHistorySize = 5;
		struct PanPoint
		{
			float offsetY;
			unsigned int ticks;
		};
		std::array<std::optional<PanPoint>, PanHistorySize> panHistory;
	public:
		ScrollPanel(Point position, Point size);

		int GetScrollLimit();
		void SetScrollPosition(int position);

		void Draw(const Point& screenPos) override;
		void XTick() override;
		void XOnMouseWheelInside(int localx, int localy, int d) override;
		void XOnMouseDown(int localx, int localy, unsigned int button) override;
		void XOnMouseUp(int x, int y, unsigned int button) override;
		void XOnMouseMoved(int localx, int localy) override;
	};
}
