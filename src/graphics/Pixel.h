#pragma once
#include <cstdint>
#include <limits>
#include <type_traits>

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

template<typename T, typename = std::enable_if<std::is_arithmetic_v<T>, void>>
struct RGB
{
	T Red, Green, Blue;

	constexpr RGB(T r, T g, T b):
		Red(r),
		Green(g),
		Blue(b)
	{
	}
};

template<typename T, typename = std::enable_if<std::is_arithmetic_v<T>, void>>
struct RGBA
{
	T Red, Green, Blue, Alpha;

	constexpr RGBA(T r, T g, T b, T a = std::numeric_limits<T>::max()):
		Red(r),
		Green(g),
		Blue(b),
		Alpha(a)
	{
	}
};
