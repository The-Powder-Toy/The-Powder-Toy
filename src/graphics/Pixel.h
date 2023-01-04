#pragma once

typedef unsigned int pixel;

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
