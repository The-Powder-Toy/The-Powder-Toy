#pragma once
#include "gui/interface/Point.h"
#include "common/Plane.h"
#include <memory>
#include <vector>

class Graphics;
class Brush
{
private:
	// 2D arrays indexed by coordinates from [-radius.X, radius.X] by [-radius.Y, radius.Y]
	PlaneAdapter<std::vector<unsigned char>> bitmap;
	PlaneAdapter<std::vector<unsigned char>> outline;

	void InitBitmap();
	void InitOutline();

	struct iterator
	{
		Brush const &parent;
		int x, y;

		iterator &operator++()
		{
			auto radius = parent.GetRadius();
			do
			{
				if (++x > radius.X)
				{
					--y;
					x = -radius.X;
				}
			} while (y >= -radius.Y && !parent.bitmap[radius + Vec2<int>{ x, y }]);
			return *this;
		}

		ui::Point operator*() const
		{
			return ui::Point(x, y);
		}

		bool operator!=(iterator other) const
		{
			return x != other.x || y != other.y;
		}

		using difference_type = void;
		using value_type = ui::Point;
		using pointer = void;
		using reference = void;
		using iterator_category = std::forward_iterator_tag;
	};

protected:
	ui::Point radius{ 0, 0 };

	virtual PlaneAdapter<std::vector<unsigned char>> GenerateBitmap() const = 0;

public:
	virtual ~Brush() = default;
	virtual void AdjustSize(int delta, bool logarithmic, bool keepX, bool keepY);
	virtual std::unique_ptr<Brush> Clone() const = 0;

	ui::Point GetSize() const
	{
		return radius * 2 + Vec2{ 1, 1 };
	}

	ui::Point GetRadius() const
	{
		return radius;
	}

	iterator begin() const
	{
		// bottom to top is the preferred order for Simulation::CreateParts
		return ++iterator{*this, radius.X, radius.Y + 1};
	}

	iterator end() const
	{
		return iterator{*this, -radius.X, -radius.Y - 1};
	}

	void RenderRect(Graphics *g, ui::Point position1, ui::Point position2) const;
	void RenderLine(Graphics *g, ui::Point position1, ui::Point position2) const;
	void RenderPoint(Graphics *g, ui::Point position) const;
	void RenderFill(Graphics *g, ui::Point position) const;

	void SetRadius(ui::Point newRadius);
};
