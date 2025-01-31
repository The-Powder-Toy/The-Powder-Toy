#pragma once
#include <cstdint>
#include <variant>
#include <optional>

namespace ui
{
	class Fade
	{
	public:
		struct LinearProfile
		{
			float change; // per second
			std::optional<float> changeDownward; // per second, ::change is upward change if set
		};
		struct ExponentialProfile
		{
			float decay; // per second
			float margin; // unit
		};
		using Profile = std::variant<
			LinearProfile,
			ExponentialProfile
		>;

	private:
		float target = 0;
		int64_t referenceTick = 0;
		float referenceValue = 0;
		Profile profile;

	public:
		Fade(Profile newProfile, float newTarget = 0.f)
		{
			SetProfile(newProfile);
			SetTarget(newTarget);
			SetValue(newTarget);
		}

		Fade(Profile newProfile, float newTarget, float newValue)
		{
			SetProfile(newProfile);
			SetTarget(newTarget);
			SetValue(newValue);
		}

		void SetTarget(float newTarget);
		void SetProfile(Profile newProfile);
		void SetValue(float newValue);
		float GetValue() const;

		operator int() const
		{
			return int(GetValue());
		}

		Fade &operator =(int newValue)
		{
			SetValue(float(newValue));
			return *this;
		}

		static constexpr ExponentialProfile BasicDimensionProfile{ 1.532496e-06f, 0.5f };
	};
}
