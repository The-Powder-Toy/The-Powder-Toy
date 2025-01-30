#pragma once
#include <cstdint>

namespace ui
{
	class Fade
	{
	public:
		int64_t minValueBiased;       // real value * 1000
		int64_t maxValueBiased;       // real value * 1000
		int64_t referenceValueBiased; // real value * 1000
		int64_t changeUpwardBiased;   // real value * 1000
		int64_t changeDownwardBiased; // real value * 1000
		bool goingUpwardThisTick = false;
		bool goingUpward = false;
		uint64_t referenceTime = 0;

		Fade(int currentValue, int minValue, int maxValue, int changeUpward, int changeDownward, int ticks)
		{
			SetRange(minValue, maxValue);
			SetRateOfChange(changeUpward, changeDownward, ticks);
			SetValue(currentValue);
		}

		void SetRange(int newMinValue, int newMaxValue);
		void SetRateOfChange(int changeUpward, int changeDownward, int ticks);
		void MarkGoingUpwardThisTick(); // this is retained until the next Tick call
		void Tick();
		void SetValue(int newValue);
		int GetValue() const;

		operator int() const
		{
			return GetValue();
		}

		Fade &operator =(int newValue)
		{
			SetValue(newValue);
			return *this;
		}
	};
}
