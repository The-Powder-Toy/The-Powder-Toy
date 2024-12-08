#pragma once

#include <algorithm>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

// This is always packed with the least significant byte being blue,
// then green, then red, then 0.
typedef uint32_t pixel;

constexpr int PIXELCHANNELS = 3;

// Least significant byte is blue, then green, then red, then alpha.
// Use sparingly, e.g. when passing packed data to a third party library.
typedef uint32_t pixel_rgba;

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
struct RGBA;

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
struct alignas(alignof(uint32_t) > alignof(T) ? alignof(uint32_t) : alignof(T)) RGB
{
	T Blue, Green, Red;

	constexpr RGB() = default;

	constexpr RGB(T r, T g, T b):
		Blue(b),
		Green(g),
		Red(r)
	{
	}

	template<typename S> // Disallow brace initialization
	RGB(std::initializer_list<S>) = delete;

	// Blend and Add get called in tight loops so it's important that they
	// vectorize well.
	template<typename S = T, typename = std::enable_if_t<std::is_same_v<S, uint8_t>>>
	constexpr RGB<T> Blend(RGBA<T> other) const
	{
		if (other.Alpha == 0xFF)
			return other.NoAlpha();
		// Dividing by 0xFF means the two branches return the same value in the
		// case that other.Alpha == 0xFF, and the division happens via
		// multiplication and bitshift anyway, so it vectorizes better than code
		// that branches in a meaningful way.
		return RGB<T>(
			// the intermediate is guaranteed to fit in 16 bits, and a 16 bit
			// multiplication vectorizes better than a longer one.
			uint16_t(other.Alpha * other.Red   + (0xFF - other.Alpha) * Red  ) / 0xFF,
			uint16_t(other.Alpha * other.Green + (0xFF - other.Alpha) * Green) / 0xFF,
			uint16_t(other.Alpha * other.Blue  + (0xFF - other.Alpha) * Blue ) / 0xFF
		);
	}

	template<typename S = T, typename = std::enable_if_t<std::is_same_v<S, uint8_t>>>
	constexpr RGB<T> Add(RGBA<T> other) const
	{
		return RGB<T>(
			std::min(0xFF, Red + uint16_t(other.Alpha * other.Red) / 0xFF),
			std::min(0xFF, Green + uint16_t(other.Alpha * other.Green) / 0xFF),
			std::min(0xFF, Blue + uint16_t(other.Alpha * other.Blue) / 0xFF)
		);
	}

	template<typename S = T, typename = std::enable_if_t<std::is_same_v<S, uint8_t>>>
	constexpr RGB<T> AddFire(RGB<T> other, int fireAlpha) const
	{
		return RGB<T>(
			std::min(0xFF, Red + (fireAlpha * other.Red) / 0xFF),
			std::min(0xFF, Green + (fireAlpha * other.Green) / 0xFF),
			std::min(0xFF, Blue + (fireAlpha * other.Blue) / 0xFF)
		);
	}

	// Decrement each component that is nonzero.
	template<typename S = T, typename = std::enable_if_t<std::is_same_v<S, uint8_t>>>
	constexpr RGB<T> Decay() const
	{
		// This vectorizes really well.
		pixel colour = Pack(), mask = colour;
		mask |= mask >> 4;
		mask |= mask >> 2;
		mask |= mask >> 1;
		mask &= 0x00010101;
		return Unpack(colour - mask);
	}

	template<typename S = T, typename = std::enable_if_t<std::is_same_v<S, uint8_t>>>
	RGB<T> Inverse() const
	{
		return RGB<T>(0xFF - Red, 0xFF - Green, 0xFF - Blue);
	}

	constexpr RGBA<T> WithAlpha(T a) const
	{
		return RGBA<T>(Red, Green, Blue, a);
	}

	template<typename S = T, typename = std::enable_if_t<std::is_same_v<S, uint8_t>>>
	constexpr pixel Pack() const
	{
		return Red << 16 | Green << 8 | Blue;
	}

	template<typename S = T, typename = std::enable_if_t<std::is_same_v<S, uint8_t>>>
	constexpr static RGB<T> Unpack(pixel px)
	{
		return RGB<T>(px >> 16, px >> 8, px);
	}
};

constexpr inline RGB<uint8_t> operator ""_rgb(unsigned long long value)
{
	return RGB<uint8_t>::Unpack(value);
}

template<typename T, typename>
struct alignas(alignof(uint32_t) > alignof(T) ? alignof(uint32_t) : alignof(T)) RGBA
{
	T Blue, Green, Red, Alpha;

	constexpr RGBA() = default;

	constexpr RGBA(T r, T g, T b, T a):
		Blue(b),
		Green(g),
		Red(r),
		Alpha(a)
	{
	}

	template<typename S = T, typename = std::enable_if_t<std::is_same_v<S, uint8_t>>>
	RGBA(T r, T g, T b):
		Blue(b),
		Green(g),
		Red(r),
		Alpha(0xFF)
	{
	}

	template<typename S> // Disallow brace initialization
	RGBA(std::initializer_list<S>) = delete;

	constexpr RGB<T> NoAlpha() const
	{
		return RGB<T>(Red, Green, Blue);
	}

	template<typename S = T, typename = std::enable_if_t<std::is_same_v<S, uint8_t>>>
	constexpr pixel Pack() const
	{
		return Red << 16 | Green << 8 | Blue | Alpha << 24;
	}

	template<typename S = T, typename = std::enable_if_t<std::is_same_v<S, uint8_t>>>
	constexpr static RGBA<T> Unpack(pixel_rgba px)
	{
		return RGBA<T>(px >> 16, px >> 8, px, px >> 24);
	}
};
