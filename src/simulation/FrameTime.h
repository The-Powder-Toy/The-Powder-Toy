#pragma once
#include "common/String.h"
#include <vector>
#include <chrono>
#include <map>
#include <optional>

class FrameTime
{
	using Clock = std::chrono::high_resolution_clock;

	std::optional<Clock::time_point> lastFrameEndAt;
	struct ActiveSpan
	{
		int retiredIndex;
		Clock::time_point begin;
	};
	std::vector<ActiveSpan> activeSpans;
	struct RetiredSpan
	{
		int level;
		const char *name;
		Clock::duration duration;
	};
	std::vector<RetiredSpan> retiredSpans;
	std::map<ByteString, double> durationAverages;
	struct AveragedSpan
	{
		int level;
		const char *name;
		double duration;
	};
	std::vector<AveragedSpan> lastAveragedSpans;

	void BeginFrame();
	void EndFrame();
	void PushSpanInner(const char *name, Clock::time_point now);
	void PushSpan(const char *name);
	void PopSpan();

public:

	const std::vector<AveragedSpan> &GetLastSpans()
	{
		return lastAveragedSpans;
	}

	class Span
	{
		FrameTime *frameTime;

	public:
		Span(FrameTime *newFrameTime, const char *name) : frameTime(newFrameTime)
		{
			if (frameTime)
			{
				frameTime->PushSpan(name);
			}
		}

		~Span()
		{
			if (frameTime)
			{
				frameTime->PopSpan();
			}
		}

		Span(const Span &) = delete;
		Span &operator =(const Span &) = delete;
	};

	struct Frame
	{
		FrameTime *frameTime;

		Frame(FrameTime *newFrameTime) : frameTime(newFrameTime)
		{
			if (frameTime)
			{
				frameTime->BeginFrame();
			}
		}

		~Frame()
		{
			if (frameTime)
			{
				frameTime->EndFrame();
			}
		}

		Frame(const Frame &) = delete;
		Frame &operator =(const Frame &) = delete;
	};
};
