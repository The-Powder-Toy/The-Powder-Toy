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
		virtual void SetProgress(int progress);
		virtual int GetProgress();
		virtual void SetStatus(String status);
		virtual String GetStatus();
		virtual void Draw(const Point & screenPos);
		virtual void Tick(float dt);
	};
}
