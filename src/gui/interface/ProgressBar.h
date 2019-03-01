#pragma once

#include "Component.h"

namespace ui
{
	class ProgressBar: public Component
	{
		int progress;
		float intermediatePos;
		String progressStatus;
	public:
	ProgressBar(Point position, Point size, int startProgress = 0, String startStatus = String());
		void SetProgress(int progress);
		int GetProgress();
		void SetStatus(String status);
		String GetStatus();
		void Draw(const Point & screenPos) override;
		void Tick(float dt) override;
	};
}
