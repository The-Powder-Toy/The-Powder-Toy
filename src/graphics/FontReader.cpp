#include "FontReader.h"

#include "bzip2/bz2wrap.h"
#include "font.bz2.h"

unsigned char *font_data = nullptr;
unsigned int *font_ptrs = nullptr;
unsigned int (*font_ranges)[2] = nullptr;

FontReader::FontReader(unsigned char const *_pointer):
	pointer(_pointer + 1),
	width(*_pointer),
	pixels(0),
	data(0)
{
}

static bool InitFontData()
{
	static std::vector<char> fontDataBuf;
	static std::vector<int> fontPtrsBuf;
	static std::vector< std::array<int, 2> > fontRangesBuf;
	if (BZ2WDecompress(fontDataBuf, reinterpret_cast<const char *>(compressed_font_data), compressed_font_data_size) != BZ2WDecompressOk)
	{
		return false;
	}
	int first = -1;
	int last = -1;
	char *begin = &fontDataBuf[0];
	char *ptr = &fontDataBuf[0];
	char *end = &fontDataBuf[0] + fontDataBuf.size();
	while (ptr != end)
	{
		if (ptr + 4 > end)
		{
			return false;
		}
		auto codePoint = *reinterpret_cast<uint32_t *>(ptr) & 0xFFFFFFU;
		if (codePoint >= 0x110000U)
		{
			return false;
		}
		auto width = *reinterpret_cast<uint8_t *>(ptr + 3);
		if (width > 64)
		{
			return false;
		}
		if (ptr + 4 + width * 3 > end)
		{
			return false;
		}
		auto cp = (int)codePoint;
		if (last >= cp)
		{
			return false;
		}
		if (first != -1 && last + 1 < cp)
		{
			fontRangesBuf.push_back({ { first, last } });
			first = -1;
		}
		if (first == -1)
		{
			first = cp;
		}
		last = cp;
		fontPtrsBuf.push_back(ptr + 3 - begin);
		ptr += width * 3 + 4;
	}
	if (first != -1)
	{
		fontRangesBuf.push_back({ { first, last } });
	}
	fontRangesBuf.push_back({ { 0, 0 } });
	font_data = reinterpret_cast<unsigned char *>(fontDataBuf.data());
	font_ptrs = reinterpret_cast<unsigned int *>(fontPtrsBuf.data());
	font_ranges = reinterpret_cast<unsigned int (*)[2]>(fontRangesBuf.data());
	return true;
}

unsigned char const *FontReader::lookupChar(String::value_type ch)
{
	if (!font_data)
	{
		if (!InitFontData())
		{
			throw std::runtime_error("font data corrupt");
		}
	}
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
