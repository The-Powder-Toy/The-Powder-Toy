#pragma once
#include <cstdint>
#include <algorithm>

class FrameSchedule
{
	uint64_t startNs = 0;
	uint64_t oldStartNs = 0;

public:
	void SetNow(uint64_t nowNs)
	{
		oldStartNs = startNs;
		startNs = nowNs;
	}

	uint64_t GetNow() const
	{
		return startNs;
	}

	uint64_t GetFrameTime() const
	{
		return startNs - oldStartNs;
	}

	uint64_t Arm(float fps)
	{
		auto oldNowNs = startNs;
		auto timeBlockDurationNs = uint64_t(std::clamp(1e9f / fps, 1.f, 1e9f));
		auto oldStartTimeBlock = oldStartNs / timeBlockDurationNs;
		auto startTimeBlock = oldStartTimeBlock + 1U;
		startNs = std::max(startNs, startTimeBlock * timeBlockDurationNs);
		return startNs - oldNowNs;
	}

	bool HasElapsed(uint64_t nowNs) const
	{
		return nowNs >= startNs;
	}
};
