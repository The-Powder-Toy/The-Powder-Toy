#pragma once
#include <cstddef>

#include "common/String.h"
#include "font.h"

class FontReader
{
	unsigned char const *pointer;
	int width;
	int pixels;
	int data;

	inline FontReader(unsigned char const *_pointer):
		pointer(_pointer + 1),
		width(*_pointer),
		pixels(0),
		data(0)
	{}

	static inline unsigned char const *lookupChar(String::value_type ch)
	{
		size_t offset = 0;
		for(int i = 0; font_ranges[i][1]; i++)
			if(font_ranges[i][0] > ch)
				break;
			else if(font_ranges[i][1] >= ch)
				return &font_data[font_ptrs[offset + (ch - font_ranges[i][0])]];
			else
				offset += font_ranges[i][1] - font_ranges[i][0] + 1;
		if(ch == 0xFFFD)
			return &font_data[0];
		else
			return lookupChar(0xFFFD);
	}

public:
	inline FontReader(String::value_type ch):
		FontReader(lookupChar(ch))
	{
	}

	inline int GetWidth() const
	{
		return width;
	}

	inline int NextPixel()
	{
		if(!pixels)
		{
			data = *(pointer++);
			pixels = 4;
		}
		int old = data;
		pixels--;
		data >>= 2;
		return old & 0x3;
	}
};
