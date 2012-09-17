#pragma once

#include "Component.h"

namespace ui
{
	class ProgressBar: public Component
	{
		int progress;
		float intermediatePos;
		std::string progressStatus;
	public:
	ProgressBar(Point position, Point size, int startProgress = 0, std::string startStatus = "");
		virtual void SetProgress(int progress);
		virtual int GetProgress();
		virtual void SetStatus(std::string status);
		virtual std::string GetStatus();
		virtual void Draw(const Point & screenPos);
		virtual void Tick(float dt);
	};	
}
