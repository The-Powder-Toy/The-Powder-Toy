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
		ProgressBar(Point position, Point size);
		virtual void SetProgress(int progress);
		virtual void SetStatus(std::string status);
		virtual void Draw(const Point & screenPos);
		virtual void Tick(float dt);
	};	
}
