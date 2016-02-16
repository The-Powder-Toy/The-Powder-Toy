#pragma once

#include "Component.h"

namespace ui
{
	class ProgressBar: public Component
	{
		int progress;
		float intermediatePos;
		std::wstring progressStatus;
	public:
	ProgressBar(Point position, Point size, int startProgress = 0, std::string startStatus = "");
	ProgressBar(Point position, Point size, int startProgress = 0, std::wstring startStatus = L"");
		virtual void SetProgress(int progress);
		virtual int GetProgress();
		virtual void SetStatus(std::string status);
		virtual void SetStatus(std::wstring status);
		virtual std::string GetStatus();
		virtual std::wstring GetWStatus();
		virtual void Draw(const Point & screenPos);
		virtual void Tick(float dt);
	};	
}
