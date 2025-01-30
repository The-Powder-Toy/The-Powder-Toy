#include "Fade.h"
#include "Engine.h"
#include <algorithm>

namespace ui
{
	constexpr int64_t bias = 1000;

	void Fade::SetRange(int newMinValue, int newMaxValue)
	{
		minValueBiased = int64_t(newMinValue) * bias;
		maxValueBiased = int64_t(newMaxValue) * bias;
		SetValue(newMinValue);
	}

	void Fade::SetRateOfChange(int changeUpward, int changeDownward, int ticks)
	{
		changeUpwardBiased = changeUpward * bias / ticks;
		changeDownwardBiased = changeDownward * bias / ticks;
	}

	void Fade::MarkGoingUpwardThisTick()
	{
		goingUpwardThisTick = true;
	}

	void Fade::Tick()
	{
		auto nextGoingUpward = goingUpwardThisTick;
		goingUpwardThisTick = false;
		if (goingUpward != nextGoingUpward)
		{
			SetValue(GetValue());
			goingUpward = nextGoingUpward;
		}
	}

	void Fade::SetValue(int newValue)
	{
		uint64_t now = Engine::Ref().LastTick();
		referenceValueBiased = newValue * bias;
		referenceTime = now;
	}

	int Fade::GetValue() const
	{
		uint64_t now = Engine::Ref().LastTick();
		auto minValueNow = goingUpward ? referenceValueBiased : minValueBiased;
		auto maxValueNow = goingUpward ? maxValueBiased : referenceValueBiased;
		auto changeNow   = goingUpward ? changeUpwardBiased : changeDownwardBiased;
		auto diff = std::min(int64_t(now - referenceTime), (maxValueNow - minValueNow) / changeNow);
		return int(goingUpward ? (minValueNow + changeNow * diff) : (maxValueNow - changeNow * diff)) / bias;
	}
}
