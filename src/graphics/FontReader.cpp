#include "FontReader.h"

FontReader::FontReader(unsigned char const *_pointer):
	pointer(_pointer + 1),
	width(*_pointer),
	pixels(0),
	data(0)
{
}

unsigned char const *FontReader::lookupChar(String::value_type ch)
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

FontReader::FontReader(String::value_type ch):
	FontReader(lookupChar(ch))
{
}

int FontReader::GetWidth() const
{
	return width;
}

int FontReader::NextPixel()
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
