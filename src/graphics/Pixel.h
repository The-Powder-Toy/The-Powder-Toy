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
constexpr int PIXELSIZE = 4;
constexpr pixel PIXPACK(int x)
{
	return x;
}
constexpr pixel PIXRGB(int r, int g, int b)
{
	return (r << 16) | (g << 8) | b;
}
constexpr int PIXR(pixel x)
{
	return (x >> 16) & 0xFF;
}
constexpr int PIXG(pixel x)
{
	return (x >> 8) & 0xFF;
}
constexpr int PIXB(pixel x)
{
	return x & 0xFF;
}

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
struct RGBA;

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
struct alignas(alignof(uint32_t) > alignof(T) ? alignof(uint32_t) : alignof(T)) RGB
{
	T Blue, Green, Red;

	constexpr RGB(T r, T g, T b):
		Blue(b),
		Green(g),
		Red(r)
	{
	}

	template<typename S> // Avoid referring to the non-intuitive order of components
	RGB(std::initializer_list<S>) = delete;

	template<typename S = T, typename = std::enable_if_t<std::is_same_v<S, uint8_t>>>
	inline RGB<T> Blend(RGBA<T> other) const
	{
		if (other.Alpha == 0xFF)
			return other.NoAlpha();
		return RGB<T>(
			// Technically should divide by 0xFF, but >> 8 is close enough
			(other.Alpha * other.Red   + (0xFF - other.Alpha) * Red  ) >> 8,
			(other.Alpha * other.Green + (0xFF - other.Alpha) * Green) >> 8,
			(other.Alpha * other.Blue  + (0xFF - other.Alpha) * Blue ) >> 8
		);
	}

	template<typename S = T, typename = std::enable_if_t<std::is_same_v<S, uint8_t>>>
	inline RGB<T> Add(RGBA<T> other) const
	{
		return RGB<T>(
			std::min(0xFF, (other.Alpha * other.Red   + 0xFF * Red  ) >> 8),
			std::min(0xFF, (other.Alpha * other.Green + 0xFF * Green) >> 8),
			std::min(0xFF, (other.Alpha * other.Blue  + 0xFF * Blue ) >> 8)
		);
	}

	template<typename S = T, typename = std::enable_if_t<std::is_same_v<S, uint8_t>>>
	inline RGB<T> Inverse() const
	{
		return RGB<T>(0xFF - Red, 0xFF - Green, 0xFF - Blue);
	}

	inline RGBA<T> WithAlpha(T a) const
	{
		return RGBA<T>(Red, Green, Blue, a);
	}

	template<typename S = T, typename = std::enable_if_t<std::is_same_v<S, uint8_t>>>
	inline pixel Pack() const
	{
		return PIXRGB(Red, Green, Blue);
	}

	template<typename S = T, typename = std::enable_if_t<std::is_same_v<S, uint8_t>>>
	static inline RGB<T> Unpack(pixel px)
	{
		return RGB<T>(PIXR(px), PIXG(px), PIXB(px));
	}
};

template<typename T, typename>
struct alignas(alignof(uint32_t) > alignof(T) ? alignof(uint32_t) : alignof(T)) RGBA
{
	T Blue, Green, Red, Alpha;

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

	template<typename S> // Avoid referring to the non-intuitive order of components
	RGBA(std::initializer_list<S>) = delete;

	RGB<T> NoAlpha() const
	{
		return RGB<T>(Red, Green, Blue);
	}
};
