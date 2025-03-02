#include "Fade.h"
#include "Engine.h"

namespace ui
{
	void Fade::SetTarget(float newTarget)
	{
		if (target == newTarget)
		{
			return;
		}
		auto value = GetValue();
		target = newTarget;
		SetValue(value);
	}

	void Fade::SetProfile(Profile newProfile)
	{
		profile = newProfile;
	}

	void Fade::SetValue(float newValue)
	{
		referenceTick = int64_t(Engine::Ref().LastTick());
		referenceValue = newValue;
	}

	float Fade::GetValue() const
	{
		constexpr auto tickBias = 1000.f;
		auto nowTick = int64_t(Engine::Ref().LastTick());
		auto diffTick = nowTick - referenceTick;
		if (auto *linearProfile = std::get_if<LinearProfile>(&profile))
		{
			auto change = linearProfile->change;
			if (target < referenceValue)
			{
				if (linearProfile->changeDownward.has_value())
				{
					change = *linearProfile->changeDownward;
				}
				change = -change;
			}
			auto maxDiffTick = int64_t((target - referenceValue) / change * tickBias);
			if (diffTick >= maxDiffTick)
			{
				return target;
			}
			return referenceValue + diffTick * change / tickBias;
		}
		if (auto *exponentialProfile = std::get_if<ExponentialProfile>(&profile))
		{
			auto maxDiffTick = int64_t(std::log(exponentialProfile->margin / std::abs(referenceValue - target)) / std::log(exponentialProfile->decay) * tickBias);
			if (diffTick >= maxDiffTick)
			{
				return target;
			}
			return target + (referenceValue - target) * std::pow(exponentialProfile->decay, diffTick / tickBias);
		}
		return 0.f;
	}
}
