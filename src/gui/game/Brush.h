#pragma once
#include "gui/interface/Point.h"
#include <memory>

class Renderer;
class Brush
{
private:
	// 2D arrays indexed by coordinates from [-radius.X, radius.X] by [-radius.Y, radius.Y]
	std::unique_ptr<unsigned char []> bitmap;
	std::unique_ptr<unsigned char []> outline;

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
			} while (y >= -radius.Y && !parent.bitmap[x + radius.X + (y + radius.Y) * (2 * radius.X + 1)]);
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

	virtual std::unique_ptr<unsigned char []> GenerateBitmap() const = 0;

public:
	Brush() = default;
	Brush(const Brush &other);
	virtual ~Brush() = default;
	virtual void AdjustSize(int delta, bool logarithmic, bool keepX, bool keepY);
	virtual std::unique_ptr<Brush> Clone() const = 0;

	ui::Point GetSize() const
	{
		return radius * 2 + 1;
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

	void RenderRect(Renderer * ren, ui::Point position1, ui::Point position2) const;
	void RenderLine(Renderer * ren, ui::Point position1, ui::Point position2) const;
	void RenderPoint(Renderer * ren, ui::Point position) const;
	void RenderFill(Renderer * ren, ui::Point position) const;

	void SetRadius(ui::Point newRadius);
};
