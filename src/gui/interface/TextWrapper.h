#pragma once

#include "common/String.h"
#include "Point.h"

#include <vector>

namespace ui
{
	class TextWrapper
	{
	public:
		struct Index
		{
			int raw_index;
			int wrapped_index;
		};

	private:
		int raw_text_size;
		String wrapped_text;
		struct clickmap_region
		{
			int pos_x, pos_y, width, pos_line;
			Index index;
		};
		int wrapped_lines;
		std::vector<clickmap_region> regions;

	public:
		int Update(String const &text, bool do_wrapping, int max_width);
		Index Raw2Index(int raw_index) const;
		Index Point2Index(int x, int y) const;
		int Index2Point(Index index, int &x, int &y) const;

		String const &WrappedText() const
		{
			return wrapped_text;
		}

		Index IndexBegin() const
		{
			return Index{ 0, 0 };
		}

		Index IndexEnd() const
		{
			return Index{ raw_text_size, (int)wrapped_text.size() };
		}
	};
}
