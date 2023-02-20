#include "Geometry.h"

// Assuming abs(dw) <= dz
template<bool Ortho, typename F>
void rasterizeLineZW(int dz, int dw, F f)
{
	const int incW = dw >= 0 ? 1 : -1;
	int z = 0, w = 0, err = 0;
	// err / (2 * dz) is the fractional error in w
	while (z <= dz)
	{
		f(z, w);

		err += 2 * dw * incW;
		if (err >= dz)
		{
			w += incW;
			err -= 2 * dz;
			if (Ortho && z < dz)
				f(z, w);
		}

		z++;
	}
}

// Ortho makes the resulting line orthogonally connected
template<bool Ortho, typename F>
void RasterizeLine(Vec2<int> p1, Vec2<int> p2, F f)
{
	if(std::abs(p1.X - p2.X) >= std::abs(p1.Y - p2.Y))
	{
		auto source = p1.X < p2.X ? p1 : p2;
		auto delta = p1.X < p2.X ? p2 - p1 : p1 - p2;
		rasterizeLineZW<Ortho>(delta.X, delta.Y, [source, f](int z, int w) { f(source + Vec2<int>(z, w)); });
	}
	else
	{
		auto source = p1.Y < p2.Y ? p1 : p2;
		auto delta = p1.Y < p2.Y ? p2 - p1 : p1 - p2;
		rasterizeLineZW<Ortho>(delta.Y, delta.X, [source, f](int z, int w) { f(source + Vec2<int>(w, z)); });
	}
}

template<typename F>
void rasterizeEllipseQuadrant(Vec2<float> radiusSquared, F f)
{
	auto inEllipse = [=](int x, int y)
	{
		return y * y * radiusSquared.X + x * x * radiusSquared.Y <= radiusSquared.X * radiusSquared.Y;
	};
	int x = int(std::floor(std::sqrt(radiusSquared.X)));
	int maxY = int(std::floor(std::sqrt(radiusSquared.Y)));
	for (int y = 0; y <= maxY; y++)
	{
		if (inEllipse(x, y + 1))
		{
			f(x, x, y);
		}
		else
		{
			int xStart = x;
			do
			{
				x--;
			} while (x >= 0 && !inEllipse(x, y + 1));
			f(x + 1, xStart, y);
		}
	}
}

template<typename F>
void RasterizeEllipsePoints(Vec2<float> radiusSquared, F f)
{
	rasterizeEllipseQuadrant(radiusSquared, [f](int x1, int x2, int y)
		{
			for (int x = x1; x <= x2; x++)
			{
				f(Vec2<int>(x, y));
				if (x) f(Vec2<int>(-x, y));
				if (y) f(Vec2<int>(x, -y));
				if (x && y) f(Vec2<int>(-x, -y));
			}
		});
}

template<typename F>
void RasterizeEllipseRows(Vec2<float> radiusSquared, F f)
{
	rasterizeEllipseQuadrant(radiusSquared, [f](int _, int xLim, int y)
		{
			f(xLim, y);
			if (y) f(xLim, -y);
		});
}
