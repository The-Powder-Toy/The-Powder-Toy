#pragma once

#include <algorithm>
#include <cstdint>
#include <initializer_list>
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

struct RGBA;

struct alignas(alignof(uint32_t)) RGB
{
	uint8_t Blue, Green, Red;

	constexpr RGB() = default;

	constexpr RGB(uint8_t r, uint8_t g, uint8_t b):
		Blue(b),
		Green(g),
		Red(r)
	{
	}

	template<typename S> // Disallow brace initialization
	RGB(std::initializer_list<S>) = delete;

	// Blend and Add get called in tight loops so it's important that they
	// vectorize well.
	constexpr RGB Blend(RGBA other) const;
	constexpr RGB Add(RGBA other) const;

	constexpr RGB AddFire(RGB other, int fireAlpha) const
	{
		return RGB(
			std::min(0xFF, Red + (fireAlpha * other.Red) / 0xFF),
			std::min(0xFF, Green + (fireAlpha * other.Green) / 0xFF),
			std::min(0xFF, Blue + (fireAlpha * other.Blue) / 0xFF)
		);
	}

	// Decrement each component that is nonzero.
	constexpr RGB Decay() const
	{
		// This vectorizes really well.
		pixel colour = Pack(), mask = colour;
		mask |= mask >> 4;
		mask |= mask >> 2;
		mask |= mask >> 1;
		mask &= 0x00010101;
		return Unpack(colour - mask);
	}

	constexpr RGB Inverse() const
	{
		return RGB(0xFF - Red, 0xFF - Green, 0xFF - Blue);
	}

	constexpr RGBA WithAlpha(uint8_t a) const;

	constexpr pixel Pack() const
	{
		return Red << 16 | Green << 8 | Blue;
	}

	constexpr static RGB Unpack(pixel px)
	{
		return RGB(px >> 16, px >> 8, px);
	}

	constexpr bool operator ==(const RGB &) const = default;
	constexpr bool operator !=(const RGB &) const = default;
};

constexpr inline RGB operator ""_rgb(unsigned long long value)
{
	return RGB::Unpack(value);
}

struct alignas(alignof(uint32_t)) RGBA
{
	uint8_t Blue, Green, Red, Alpha;

	constexpr RGBA() = default;

	constexpr RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a):
		Blue(b),
		Green(g),
		Red(r),
		Alpha(a)
	{
	}

	RGBA(uint8_t r, uint8_t g, uint8_t b):
		Blue(b),
		Green(g),
		Red(r),
		Alpha(0xFF)
	{
	}

	template<typename S> // Disallow brace initialization
	RGBA(std::initializer_list<S>) = delete;

	constexpr RGBA Blend(RGBA other) const;
	constexpr RGBA Add(RGBA other) const;

	constexpr RGB NoAlpha() const
	{
		return RGB(Red, Green, Blue);
	}

	constexpr pixel Pack() const
	{
		return Red << 16 | Green << 8 | Blue | Alpha << 24;
	}

	constexpr static RGBA Unpack(pixel_rgba px)
	{
		return RGBA(px >> 16, px >> 8, px, px >> 24);
	}

	constexpr bool operator ==(const RGBA &) const = default;
	constexpr bool operator !=(const RGBA &) const = default;
};

// Blend and Add get called in tight loops so it's important that they
// vectorize well.
constexpr RGB RGB::Blend(RGBA other) const
{
	if (other.Alpha == 0xFF)
		return other.NoAlpha();
	// Dividing by 0xFF means the two branches return the same value in the
	// case that other.Alpha == 0xFF, and the division happens via
	// multiplication and bitshift anyway, so it vectorizes better than code
	// that branches in a meaningful way.
	return RGB(
		// the intermediate is guaranteed to fit in 16 bits, and a 16 bit
		// multiplication vectorizes better than a longer one.
		uint16_t(other.Alpha * other.Red   + (0xFF - other.Alpha) * Red  ) / 0xFF,
		uint16_t(other.Alpha * other.Green + (0xFF - other.Alpha) * Green) / 0xFF,
		uint16_t(other.Alpha * other.Blue  + (0xFF - other.Alpha) * Blue ) / 0xFF
	);
}

constexpr RGBA RGBA::Blend(RGBA other) const
{
	if (other.Alpha == 0xFF)
	{
		return other;
	}
	// blending as per the following matrix:
	//   [ 1-a   0    0   ar ]
	//   [  0   1-a   0   ag ]
	//   [  0    0   1-a  ab ]
	//   [  0    0    0   1  ]
	auto outAlpha = 0xFE01 - (0xFF - other.Alpha) * (0xFF - Alpha);
	auto outRed   = outAlpha ? ((0xFF - other.Alpha) * Alpha * Red   + 0xFF * other.Alpha * other.Red  ) / outAlpha : 0;
	auto outGreen = outAlpha ? ((0xFF - other.Alpha) * Alpha * Green + 0xFF * other.Alpha * other.Green) / outAlpha : 0;
	auto outBlue  = outAlpha ? ((0xFF - other.Alpha) * Alpha * Blue  + 0xFF * other.Alpha * other.Blue ) / outAlpha : 0;
	outAlpha /= 0xFF;
	return RGBA(
		uint8_t(outRed),
		uint8_t(outGreen),
		uint8_t(outBlue),
		uint8_t(outAlpha)
	);
}

constexpr RGB RGB::Add(RGBA other) const
{
	return RGB(
		std::min(0xFF, Red + uint16_t(other.Alpha * other.Red) / 0xFF),
		std::min(0xFF, Green + uint16_t(other.Alpha * other.Green) / 0xFF),
		std::min(0xFF, Blue + uint16_t(other.Alpha * other.Blue) / 0xFF)
	);
}

constexpr RGBA RGBA::Add(RGBA other) const
{
	return RGBA(
		std::min(0xFF, Red + uint16_t(other.Alpha * other.Red) / 0xFF),
		std::min(0xFF, Green + uint16_t(other.Alpha * other.Green) / 0xFF),
		std::min(0xFF, Blue + uint16_t(other.Alpha * other.Blue) / 0xFF),
		std::min(0xFF, Alpha + uint16_t(other.Alpha * other.Alpha) / 0xFF)
	);
}

constexpr RGBA RGB::WithAlpha(uint8_t a) const
{
	return RGBA(Red, Green, Blue, a);
}

constexpr inline RGBA operator ""_argb(unsigned long long value)
{
	return RGBA::Unpack(value);
}
