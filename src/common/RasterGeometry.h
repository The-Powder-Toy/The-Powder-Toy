#include "common/Vec2.h"

// Draw a line from the origin on the ZW plane, assuming abs(dw) <= dz
template<bool Ortho, typename F>
void rasterizeLineZW(int dz, int dw, F f)
{
	const int incW = dw >= 0 ? 1 : -1;
	int w = 0, err = 0;
	for (int z = 0; z <= dz; z++)
	{
		f(z, w);

		// err / (2 * dz) is the difference between the integer w and the
		// (potentially non-integer) value z * dw / dz that would like w to be.
		// When the difference becomes too large, we can increment w.
		err += 2 * dw * incW;
		if (err >= dz)
		{
			w += incW;
			err -= 2 * dz;
			if (Ortho && z < dz)
				f(z, w);
		}
	}
}

// Call f for every point on the rasterization of a line between p1 and p2.
// Ortho makes the resulting line orthogonally connected.
template<bool Ortho, typename F>
void RasterizeLine(Vec2<int> p1, Vec2<int> p2, F f)
{
	if(std::abs(p1.X - p2.X) >= std::abs(p1.Y - p2.Y))
	{
		// If it's more wide than tall, map Z to X and W to Y
		auto source = p1.X < p2.X ? p1 : p2;
		auto delta = p1.X < p2.X ? p2 - p1 : p1 - p2;
		rasterizeLineZW<Ortho>(delta.X, delta.Y, [source, f](int z, int w) { f(source + Vec2(z, w)); });
	}
	else
	{
		// If it's more tall than wide, map Z to Y and W to X
		auto source = p1.Y < p2.Y ? p1 : p2;
		auto delta = p1.Y < p2.Y ? p2 - p1 : p1 - p2;
		rasterizeLineZW<Ortho>(delta.Y, delta.X, [source, f](int z, int w) { f(source + Vec2(w, z)); });
	}
}

template<typename F>
void rasterizeEllipseQuadrant(Vec2<float> radiusSquared, F f)
{
	// An ellipse is a region of points (x, y) such that
	// (x / rx)^2 + (y / ry)^2 <= 1, which can be rewritten as
	// x^2 * ry^2 + y^2 * rx^2 <= ry^2 * rx^2,
	// except, if rx == 0, then an additional constraint abs(y) <= ry must be
	// added, and same for ry.
	// The code below ensures 0 <= x <= rx and 0 <= y <= ry + 1.
	// A false positive for y > ry can only happen if rx == 0 and does not
	// affect the outcome
	auto inEllipse = [=](int x, int y)
	{
		return y * y * radiusSquared.X + x * x * radiusSquared.Y <= radiusSquared.X * radiusSquared.Y;
	};
	// Focusing on the bottom right quadrant, in every row we find the range of
	// points inside the ellipse, and within those, the range of points on the
	// boundary.
	int x = int(std::floor(std::sqrt(radiusSquared.X)));
	int maxY = int(std::floor(std::sqrt(radiusSquared.Y)));
	for (int y = 0; y <= maxY; y++)
	{
		// At the start of each iteration, (x, y) is on the boundary,
		// i.e. the range of points inside the ellipse is [0, x]
		if (inEllipse(x, y + 1))
		{
			// If the point below is inside, x is the only boundary point
			f(x, x, y);
		}
		else
		{
			// Otherwise, all points whose below point is outside -- are on the boundary
			int xStart = x;
			do
			{
				x--;
			} while (x >= 0 && !inEllipse(x, y + 1));
			f(x + 1, xStart, y);
		}
	}
}

// Call f for every point on the rasterized boundary of the ellipse with the
// indicated radius.
// In some situations we may want the radius to be the square root of an
// integer, so passing the radius squared allows for exact calculation.
// In some situations we may want the radius to be a half-integer, and floating
// point arithmetic is still exact for half-integers (really, 1/16ths), which is
// why we pass this as a float.
template<typename F>
void RasterizeEllipsePoints(Vec2<float> radiusSquared, F f)
{
	rasterizeEllipseQuadrant(radiusSquared, [f](int x1, int x2, int y)
		{
			for (int x = x1; x <= x2; x++)
			{
				f(Vec2(x, y));
				if (x) f(Vec2(-x, y));
				if (y) f(Vec2(x, -y));
				if (x && y) f(Vec2(-x, -y));
			}
		});
}

// Call f for every point inside the ellipse with the indicated radius.
template<typename F>
void RasterizeEllipseRows(Vec2<float> radiusSquared, F f)
{
	rasterizeEllipseQuadrant(radiusSquared, [f](int _, int xLim, int y)
		{
			f(xLim, y);
			if (y) f(xLim, -y);
		});
}

// Call f for every point on the boundary of the indicated rectangle (so that
// pos and BottomRight are both corners).
template<typename F>
void RasterizeRect(Rect<int> rect, F f)
{
	for (int x = rect.pos.X; x < rect.pos.X + rect.size.X; x++)
		f(Vec2(x, rect.pos.Y));

	if (rect.pos.Y != rect.pos.Y + rect.size.Y - 1)
		for (int x = rect.pos.X; x < rect.pos.X + rect.size.X; x++)
			f(Vec2(x, rect.pos.Y + rect.size.Y - 1));

	// corners already drawn
	for (int y = rect.pos.Y + 1; y < rect.pos.Y + rect.size.Y - 1; y++)
		f(Vec2(rect.pos.X, y));

	if (rect.pos.X != rect.pos.X + rect.size.X - 1)
		for (int y = rect.pos.Y + 1; y < rect.pos.Y + rect.size.Y - 1; y++)
			f(Vec2(rect.pos.X + rect.size.X - 1, y));
}

// Call f for every point on the dotted boundary of the indicated rectangle.
template<typename F>
void RasterizeDottedRect(Rect<int> rect, F f)
{
	for (int x = rect.pos.X; x < rect.pos.X + rect.size.X; x += 2)
		f(Vec2(x, rect.pos.Y));

	int bottomOff = (rect.pos.Y + rect.size.Y - 1 - rect.pos.Y) % 2;
	if (rect.pos.Y != rect.pos.Y + rect.size.Y - 1)
		for (int x = rect.pos.X + bottomOff; x < rect.pos.X + rect.size.X; x += 2)
			f(Vec2(x, rect.pos.Y + rect.size.Y - 1));

	// corners already drawn
	for (int y = rect.pos.Y + 1 + 1; y < rect.pos.Y + rect.size.Y - 1; y += 2)
		f(Vec2(rect.pos.X, y));

	int leftOff = (rect.pos.X + rect.size.X - 1 - rect.pos.X + 1) % 2;
	if (rect.pos.X != rect.pos.X + rect.size.X - 1)
		for (int y = rect.pos.Y + 1 + leftOff; y < rect.pos.Y + rect.size.Y - 1; y += 2)
			f(Vec2(rect.pos.X + rect.size.X - 1, y));
}
