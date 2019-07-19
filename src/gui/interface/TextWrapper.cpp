#include "TextWrapper.h"

#include "graphics/Graphics.h"
#include "graphics/FontReader.h"

#include <algorithm>
#include <vector>
#include <iterator>

namespace ui
{
	int TextWrapper::Update(String const &text, bool do_wrapping, int max_width)
	{
		raw_text_size = (int)text.size();

		struct wrap_record
		{
			String::value_type character;
			int width;
			std::iterator_traits<String::iterator>::difference_type position;
			bool wraps;
		};
		int line_width = 0;
		std::vector<wrap_record> records;

		int word_begins_at = -1; // this is a pointer into records; we're not currently in a word
		int word_width;
		int lines = 1;
		for (auto it = text.begin(); it != text.end(); ++it)
		{
			auto char_width = Graphics::CharWidth(*it);

			int sequence_length = 0;
			switch (*it) // set sequence_length if *it starts a sequence that should be forwarded as-is
			{
			case   '\b': sequence_length = 2; break;
			case '\x0f': sequence_length = 4; break;
			}
			
			switch (*it)
			{
			// add more supported spaces here
			case ' ':
				if (do_wrapping && line_width + char_width > max_width)
				{
					records.push_back(wrap_record{
						'\n', // character; makes the line wrap when rendered
						0, // width; fools the clickmap generator into not seeing this newline
						0, // position; the clickmap generator is fooled, this can be anything
						true // signal the end of the line to the clickmap generator
					});
					line_width = 0;
					lines += 1;
				}
				else
				{
					// this is in an else branch to make spaces immediately following
					// newline characters inserted by the wrapper disappear
					records.push_back(wrap_record{
						*it,
						char_width,
						it - text.begin(),
						false
					});
					line_width += char_width;
				}
				word_begins_at = -1; // reset word state
				break;

			// add more supported linebreaks here
			case '\n':
				records.push_back(wrap_record{
					*it, // character; makes the line wrap when rendered
					max_width - line_width, // width; make it span all the way to the end
					it - text.begin(), // position; so the clickmap generator knows where *it is
					true // signal the end of the line to the clickmap generator
				});
				lines += 1;
				line_width = 0;
				word_begins_at = -1; // reset word state
				break;

			default:
				if (sequence_length) // *it starts a sequence such as \b? or \x0f???
				{
					if (text.end() - it < sequence_length)
					{
						it = text.end() - 1;
						continue; // text is broken, we might as well skip the whole thing
					}
					for (auto skip = it + sequence_length; it != skip; ++it)
					{
						records.push_back(wrap_record{
							*it, // character; forward the sequence to the output
							0, // width; fools the clickmap generator into not seeing this sequence
							0, // position; the clickmap generator is fooled, this can be anything
							false // signal nothing to the clickmap generator
						});
					}
					--it;
				}
				else
				{
					if (word_begins_at == -1)
					{
						word_begins_at = records.size();
						word_width = 0;
					}

					if (do_wrapping && word_width + char_width > max_width)
					{
						records.push_back(wrap_record{
							'\n', // character; makes the line wrap when rendered
							0, // width; fools the clickmap generator into not seeing this newline
							0, // position; the clickmap generator is fooled, this can be anything
							true // signal the end of the line to the clickmap generator
						});
						lines += 1;
						word_begins_at = records.size();
						word_width = 0;
						line_width = 0;
					}
					if (do_wrapping && line_width + char_width > max_width)
					{
						// if we get in here, we skipped the previous block (since line_width
						// would have been set to 0 (unless of course (char_width > max_width) which
						// is dumb)). since (word_width + char_width) <= (line_width + char_width) always
						// holds and we are in this block, we can be sure that word_width < line_width,
						// so breaking the line by the preceding space is sure to decrease line_width.
						records.insert(records.begin() + word_begins_at, wrap_record{
							'\n', // character; makes the line wrap when rendered
							0, // width; fools the clickmap generator into not seeing this newline
							0, // position; the clickmap generator is fooled, this can be anything
							true // signal the end of the line to the clickmap generator
						});
						lines += 1;
						word_begins_at += 1;
						line_width = word_width;
					}

					records.push_back(wrap_record{
						*it, // character; make the line wrap with *it
						char_width, // width; make it span all the way to the end
						it - text.begin(), // position; so the clickmap generator knows where *it is
						false // signal nothing to the clickmap generator
					});
					word_width += char_width;
					line_width += char_width;
				}
				break;
			}
		}

		regions.clear();
		wrapped_text.clear();
		int x = 0;
		int l = 0;
		int counter = 0;
		for (auto const &record : records)
		{
			regions.push_back(clickmap_region{ x, l * FONT_H, record.width, l + 1, Index{ (int)record.position, counter } });
			++counter;
			x += record.width;
			if (record.wraps)
			{
				x = 0;
				l += 1;
			}
			wrapped_text.append(1, record.character);
		}

		wrapped_lines = lines;
		return lines;
	}

	TextWrapper::Index TextWrapper::Point2Index(int x, int y) const
	{
		if (y < 0)
		{
			return IndexBegin();
		}
		if (regions.size())
		{
			auto curr = regions.begin();
			auto end = regions.end();

			auto find_next_nonempty = [end](decltype(end) it) {
				++it;
				while (it != end && !it->width)
				{
					++it;
				}
				return it;
			};

			auto next = find_next_nonempty(curr);
			while (next != end)
			{
				if (curr->pos_y + FONT_H > y)
				{
					if (curr->pos_x + curr->width / 2 > x)
					{
						// if x is to the left of the vertical bisector of the current region,
						// return this one; really we should have returned 'the next one' in
						// the previous iteration
						return curr->index;
					}
					if (curr->pos_x + curr->width / 2 <= x && next->pos_x + next->width / 2 > x)
					{
						// if x is to the right of the vertical bisector of the current region
						// but to the left of the next one's, return the next one
						return next->index;
					}
					if (curr->pos_x + curr->width / 2 <= x && next->pos_y > curr->pos_y)
					{
						// nominate the next region if x is to the right of the vertical bisector of
						// the current region and the next one is on a new line
						return next->index;
					}
				}
				curr = next;
				next = find_next_nonempty(next);
			}
		}
		return IndexEnd();
	}

	int TextWrapper::Index2Point(Index index, int &x, int &y) const
	{
		if (index.wrapped_index < 0 || index.wrapped_index > (int)regions.size() || !regions.size())
		{
			return -1;
		}
		if (index.wrapped_index == (int)regions.size())
		{
			x = regions[index.wrapped_index - 1].pos_x + regions[index.wrapped_index - 1].width;
			y = regions[index.wrapped_index - 1].pos_y;
			return regions[index.wrapped_index - 1].pos_line;
		}
		x = regions[index.wrapped_index].pos_x;
		y = regions[index.wrapped_index].pos_y;
		return regions[index.wrapped_index].pos_line;
	}

	TextWrapper::Index TextWrapper::Raw2Index(int raw_index) const
	{
		if (raw_index < 0)
		{
			return IndexBegin();
		}
		for (auto const &region : regions)
		{
			if (region.index.raw_index >= raw_index)
			{
				return region.index;
			}
		}
		return IndexEnd();
	}
}
