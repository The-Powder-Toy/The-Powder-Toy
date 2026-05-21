#include "FrameTime.h"
#include "common/Assert.h"
#include <string.h>

void FrameTime::BeginFrame()
{
	PushSpanInner("Frame time", lastFrameEndAt ? *lastFrameEndAt : Clock::now());
}

void FrameTime::EndFrame()
{
	PopSpan();
	assert(activeSpans.empty());
	lastAveragedSpans.clear();
	std::map<ByteString, double> lastDurationAverages;
	std::vector<AveragedSpan> averagedSpans;
	std::swap(durationAverages, lastDurationAverages);
	for (auto &span : retiredSpans)
	{
		auto currDuration = double(std::chrono::duration_cast<std::chrono::nanoseconds>(span.duration).count());
		auto prevDuration = lastDurationAverages[span.name];
		auto duration = prevDuration + (currDuration - prevDuration) * 0.05;
		durationAverages[span.name] = duration;
		averagedSpans.push_back({ span.level, span.name, duration });
	}
	retiredSpans.clear();
	std::swap(averagedSpans, lastAveragedSpans);
	lastFrameEndAt = Clock::now();
}

void FrameTime::PushSpanInner(const char *name, Clock::time_point now)
{
	retiredSpans.push_back({ int(activeSpans.size()), name, {} });
	activeSpans.push_back({ int(retiredSpans.size()) - 1, now });
}

void FrameTime::PushSpan(const char *name)
{
	assert(!activeSpans.empty());
	PushSpanInner(name, Clock::now());
}

void FrameTime::PopSpan()
{
	assert(!activeSpans.empty());
	auto now = Clock::now();
	retiredSpans[activeSpans.back().retiredIndex].duration = now - activeSpans.back().begin;
	activeSpans.pop_back();
}
