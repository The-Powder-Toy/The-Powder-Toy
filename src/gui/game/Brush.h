#pragma once
#include "gui/interface/Point.h"
#include <memory>

class Renderer;
class Brush
{
private:
	ui::Point mutable size;
	// 2D arrays indexed by coordinates from [-size.X, size.X] by [-size.Y, size.Y]
	std::unique_ptr<unsigned char []> mutable bitmap;
	std::unique_ptr<unsigned char []> mutable outline;

	void ensureBitmap() const;
	void ensureOutline() const;

	struct iterator
	{
		Brush const &parent;
		int x, y;

		iterator &operator++()
		{
			do
			{
				if (++x > parent.size.X)
				{
					--y;
					x = -parent.size.X;
				}
			} while (y >= -parent.size.Y && !parent.bitmap[x + parent.size.X + (y + parent.size.Y) * (2 * parent.size.X + 1)]);
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
	Brush():
		size(0, 0),
		bitmap(),
		outline()
	{
	}

	void InvalidateCache();
	virtual std::pair<ui::Point, std::unique_ptr<unsigned char []>> GenerateBitmap() const = 0;
	void copyBitmaps(Brush &into) const;

public:
	virtual ~Brush() = default;
	virtual ui::Point GetRadius() const = 0;
	virtual void SetRadius(ui::Point radius) = 0;
	virtual void AdjustSize(int delta, bool logarithmic, bool keepX, bool keepY);
	virtual std::unique_ptr<Brush> Clone() const = 0;

	ui::Point GetSize() const
	{
		return size;
	}

	iterator begin() const
	{
		// bottom to top is the preferred order for Simulation::CreateParts
		return ++iterator{*this, size.X, size.Y + 1};
	}

	iterator end() const
	{
		return iterator{*this, -size.X, -size.Y - 1};
	}

	void RenderRect(Renderer * ren, ui::Point position1, ui::Point position2) const;
	void RenderLine(Renderer * ren, ui::Point position1, ui::Point position2) const;
	void RenderPoint(Renderer * ren, ui::Point position) const;
	void RenderFill(Renderer * ren, ui::Point position) const;
};
