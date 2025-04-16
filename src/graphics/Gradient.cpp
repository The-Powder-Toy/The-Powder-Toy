#include "Gradient.h"

std::vector<RGB> Gradient(std::vector<GradientStop> stops, int resolution)
{
	std::vector<RGB> table(resolution, 0x000000_rgb);
	if (stops.size() >= 2)
	{
		std::sort(stops.begin(), stops.end());
		auto stop = -1;
		for (auto i = 0; i < resolution; ++i)
		{
			auto point = i / (float)resolution;
			while (stop < (int)stops.size() - 1 && stops[stop + 1].point <= point)
			{
				++stop;
			}
			if (stop < 0 || stop >= (int)stops.size() - 1)
			{
				continue;
			}
			auto &left = stops[stop];
			auto &right = stops[stop + 1];
			auto f = (point - left.point) / (right.point - left.point);
			table[i] = left.color.Blend(right.color.WithAlpha(uint8_t(f * 0xFF)));
		}
	}
	return table;
}
