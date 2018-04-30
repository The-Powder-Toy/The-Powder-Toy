#include <cstddef>

#include "common/String.h"
#include "font.h"

class FontReader
{
	unsigned char *pointer;
	int width;
	int pixels;
	int data;

	inline FontReader(unsigned char *_pointer):
		pointer(_pointer + 1),
		width(*_pointer),
		pixels(0),
		data(0)
	{}

public:
	inline FontReader(String::value_type ch):
		FontReader(ch <= 0xFF ? &font_data[font_ptrs[ch]] : &font_data[0])
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
