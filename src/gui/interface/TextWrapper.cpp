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
			int raw_index;
			int clear_index;
			bool wraps;
			bool may_eat_space;
		};
		int line_width = 0;
		std::vector<wrap_record> records;

		int word_begins_at = -1; // this is a pointer into records; we're not currently in a word
		int word_width;
		int lines = 1;
		int char_width;
		int clear_count = 0;

		auto wrap_if_needed = [&](int width_to_consider) -> bool {
			if (do_wrapping && width_to_consider + char_width > max_width)
			{
				records.push_back(wrap_record{
					'\n', // character; makes the line wrap when rendered
					0, // width; fools the clickmap generator into not seeing this newline
					0, // position; the clickmap generator is fooled, this can be anything
					0,
					true, // signal the end of the line to the clickmap generator
					true // allow record to eat the following space
				});
				line_width = 0;
				lines += 1;
				return true;
			}
			return false;
		};

		for (auto it = text.begin(); it != text.end(); ++it)
		{
			char_width = Graphics::CharWidth(*it);

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
				wrap_if_needed(line_width);
				if (records.size() && records.back().may_eat_space)
				{
					records.back().may_eat_space = false;
				}
				else
				{
					// this is pushed only if the previous record isn't a wrapping one
					// to make spaces immediately following newline characters inserted
					// by the wrapper disappear
					records.push_back(wrap_record{
						*it,
						char_width,
						(int)(it - text.begin()),
						clear_count,
						false,
						false
					});
					line_width += char_width;
				}
				word_begins_at = -1; // reset word state
				++clear_count;
				break;

			// add more supported linebreaks here
			case '\n':
				records.push_back(wrap_record{
					*it, // character; makes the line wrap when rendered
					max_width - line_width, // width; make it span all the way to the end
					(int)(it - text.begin()), // position; so the clickmap generator knows where *it is
					clear_count,
					true, // signal the end of the line to the clickmap generator
					false
				});
				lines += 1;
				line_width = 0;
				word_begins_at = -1; // reset word state
				++clear_count;
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
							0,
							false, // signal nothing to the clickmap generator
							false
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

					if (wrap_if_needed(word_width))
					{
						word_begins_at = records.size();
						word_width = 0;
					}
					if (wrap_if_needed(line_width))
					{
						// if we get in here, we skipped the previous block (since line_width
						// would have been set to 0 (unless of course (char_width > max_width) which
						// is dumb)). since (word_width + char_width) <= (line_width + char_width) always
						// holds and we are in this block, we can be sure that word_width < line_width,
						// so breaking the line by the preceding space is sure to decrease line_width.

						// now of course there's this problem that wrap_if_needed appends the
						// newline character to the end of records, and we want it before
						// the record at position word_begins_at (0-based)
						std::rotate(
							records.begin() + word_begins_at,
							records.end() - 1,
							records.end()
						);
						word_begins_at += 1;
						line_width = word_width;
					}

					records.push_back(wrap_record{
						*it, // character; make the line wrap with *it
						char_width, // width; make it span all the way to the end
						(int)(it - text.begin()), // position; so the clickmap generator knows where *it is
						clear_count,
						false, // signal nothing to the clickmap generator
						false
					});
					word_width += char_width;
					line_width += char_width;
					++clear_count;

					switch (*it)
					{
					// add more supported non-spaces here that break the word
					case '?':
					case ';':
					case ',':
					case ':':
					case '.':
					case '-':
					case '!':
						word_begins_at = -1; // reset word state
						break;
					}
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
			regions.push_back(clickmap_region{ x, l * FONT_H, record.width, l + 1, Index{ record.raw_index, counter, record.clear_index } });
			++counter;
			x += record.width;
			if (record.wraps)
			{
				x = 0;
				l += 1;
			}
			wrapped_text.append(1, record.character);
		}

		clear_text_size = clear_count;
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

	TextWrapper::Index TextWrapper::Clear2Index(int clear_index) const
	{
		if (clear_index < 0)
		{
			return IndexBegin();
		}
		for (auto const &region : regions)
		{
			if (region.index.clear_index >= clear_index)
			{
				return region.index;
			}
		}
		return IndexEnd();
	}
}
