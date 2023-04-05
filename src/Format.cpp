#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iostream>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <png.h>
#include "Format.h"
#include "graphics/Graphics.h"

ByteString format::UnixtimeToDate(time_t unixtime, ByteString dateFormat)
{
	struct tm * timeData;
	char buffer[128];

	timeData = localtime(&unixtime);

	strftime(buffer, 128, dateFormat.c_str(), timeData);
	return ByteString(buffer);
}

ByteString format::UnixtimeToDateMini(time_t unixtime)
{
	time_t currentTime = time(NULL);
	struct tm currentTimeData = *gmtime(&currentTime);
	struct tm timeData = *gmtime(&unixtime);

	if(currentTimeData.tm_year != timeData.tm_year)
	{
		return UnixtimeToDate(unixtime, "%d %b %Y");
	}
	else if(currentTimeData.tm_mon != timeData.tm_mon || currentTimeData.tm_mday != timeData.tm_mday)
	{
		return UnixtimeToDate(unixtime, "%d %B");
	}
	else
	{
		return UnixtimeToDate(unixtime, "%H:%M:%S");
	}
}

String format::CleanString(String dirtyString, bool ascii, bool color, bool newlines, bool numeric)
{
	for (size_t i = 0; i < dirtyString.size(); i++)
	{
		switch(dirtyString[i])
		{
		case '\b':
			if (color)
			{
				dirtyString.erase(i, 2);
				i--;
			}
			else
				i++;
			break;
		case '\x0E':
			if (color)
			{
				dirtyString.erase(i, 1);
				i--;
			}
			break;
		case '\x0F':
			if (color)
			{
				dirtyString.erase(i, 4);
				i--;
			}
			else
				i += 3;
			break;
		case '\r':
		case '\n':
			if (newlines)
				dirtyString[i] = ' ';
			break;
		default:
			if (numeric && (dirtyString[i] < '0' || dirtyString[i] > '9'))
			{
				dirtyString.erase(i, 1);
				i--;
			}
			// if less than ascii 20 or greater than ascii 126, delete
			else if (ascii && (dirtyString[i] < ' ' || dirtyString[i] > '~'))
			{
				dirtyString.erase(i, 1);
				i--;
			}
			break;
		}
	}
	return dirtyString;
}

std::vector<char> format::PixelsToPPM(PlaneAdapter<std::vector<pixel>> const &input)
{
	std::vector<char> data;
	char buffer[256];
	sprintf(buffer, "P6\n%d %d\n255\n", input.Size().X, input.Size().Y);
	data.insert(data.end(), buffer, buffer + strlen(buffer));

	data.reserve(data.size() + input.Size().X * input.Size().Y * 3);

	for (int i = 0; i < input.Size().X * input.Size().Y; i++)
	{
		auto colour = RGB<uint8_t>::Unpack(input.data()[i]);
		data.push_back(colour.Red);
		data.push_back(colour.Green);
		data.push_back(colour.Blue);
	}

	return data;
}

static std::unique_ptr<PlaneAdapter<std::vector<uint32_t>>> readPNG(
	std::vector<char> const &data,
	// If omitted,
	//   RGB data is returned with A=0xFF
	//   RGBA data is returned as itself
	// If specified
	//   RGB data is returned with A=0x00
	//   RGBA data is blended against the background and returned with A=0x00
	std::optional<RGB<uint8_t>> background
)
{
	png_infop info = nullptr;
	auto deleter = [&info](png_struct *png) {
		png_destroy_read_struct(&png, &info, NULL);
	};
	auto png = std::unique_ptr<png_struct, decltype(deleter)>(
		png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
			[](png_structp png, png_const_charp msg) {
				fprintf(stderr, "PNG error: %s\n", msg);
			},
			[](png_structp png, png_const_charp msg) {
				fprintf(stderr, "PNG warning: %s\n", msg);
			}
		), deleter
	);
	if (!png)
		return nullptr;

	// libpng might longjmp() here in case of error
	// Every time we create an object with a non-trivial destructor we must call setjmp again
	if (setjmp(png_jmpbuf(png.get())))
		return nullptr;

	info = png_create_info_struct(png.get());
	if (!info)
		return nullptr;

	auto it = data.begin();
	auto const end = data.end();
	auto readFn = [&it, end](png_structp png, png_bytep data, size_t length) {
		if (size_t(end - it) < length)
			png_error(png, "Tried to read beyond the buffer");
		std::copy_n(it, length, data);
		it += length;
	};

	// See above
	if (setjmp(png_jmpbuf(png.get())))
		return nullptr;

	png_set_read_fn(png.get(), static_cast<void *>(&readFn), [](png_structp png, png_bytep data, size_t length) {
		(*static_cast<decltype(readFn) *>(png_get_io_ptr(png)))(png, data, length);
	});
	png_set_user_limits(png.get(), RES.X, RES.Y); // Refuse to parse larger images
	png_read_info(png.get(), info);

	auto output = std::make_unique<PlaneAdapter<std::vector<uint32_t>>>(
		Vec2<int>(png_get_image_width(png.get(), info), png_get_image_height(png.get(), info))
	);

	std::vector<png_bytep> rowPointers(output->Size().Y);
	for (int y = 0; y < output->Size().Y; y++)
		rowPointers[y] = reinterpret_cast<png_bytep>(&*output->RowIterator(Vec2(0, y)));

	// See above
	if (setjmp(png_jmpbuf(png.get())))
		return nullptr;

	png_set_filler(png.get(), background ? 0x00 : 0xFF, PNG_FILLER_AFTER);
	png_set_bgr(png.get());

	auto bitDepth = png_get_bit_depth(png.get(), info);
	auto colorType = png_get_color_type(png.get(), info);
	if (colorType == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png.get());
	if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
		png_set_expand_gray_1_2_4_to_8(png.get());
	if (bitDepth == 16)
		png_set_scale_16(png.get());
	if (png_get_valid(png.get(), info, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png.get());
	if (colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png.get());
	if (background)
	{
		png_color_16 colour;
		colour.red = background->Red;
		colour.green = background->Green;
		colour.blue = background->Blue;
		png_set_background(png.get(), &colour, PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);
	}

	png_read_image(png.get(), rowPointers.data());
	return output;
}

std::unique_ptr<PlaneAdapter<std::vector<pixel_rgba>>> format::PixelsFromPNG(std::vector<char> const &data)
{
	return readPNG(data, std::nullopt);
}

std::unique_ptr<PlaneAdapter<std::vector<pixel>>> format::PixelsFromPNG(std::vector<char> const &data, RGB<uint8_t> background)
{
	return readPNG(data, background);
}

std::unique_ptr<std::vector<char>> format::PixelsToPNG(PlaneAdapter<std::vector<pixel>> const &input)
{
	png_infop info = nullptr;
	auto deleter = [&info](png_struct *png) {
		png_destroy_write_struct(&png, &info);
	};
	auto png = std::unique_ptr<png_struct, decltype(deleter)>(
		png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL,
			[](png_structp png, png_const_charp msg) {
				fprintf(stderr, "PNG error: %s\n", msg);
			},
			[](png_structp png, png_const_charp msg) {
				fprintf(stderr, "PNG warning: %s\n", msg);
			}
		), deleter
	);
	if (!png)
		return nullptr;

	// libpng might longjmp() here in case of error
	// Every time we create an object with a non-trivial destructor we must call setjmp again
	if (setjmp(png_jmpbuf(png.get())))
		return nullptr;


	info = png_create_info_struct(png.get());
	if (!info)
		return nullptr;

	std::vector<char> output;
	auto writeFn = [&output](png_structp png, png_bytep data, size_t length) {
		output.insert(output.end(), data, data + length);
	};

	std::vector<png_const_bytep> rowPointers(input.Size().Y);
	for (int y = 0; y < input.Size().Y; y++)
		rowPointers[y] = reinterpret_cast<png_const_bytep>(&*input.RowIterator(Vec2(0, y)));

	// See above
	if (setjmp(png_jmpbuf(png.get())))
		return nullptr;

	png_set_write_fn(png.get(), static_cast<void *>(&writeFn), [](png_structp png, png_bytep data, size_t length) {
		(*static_cast<decltype(writeFn) *>(png_get_io_ptr(png)))(png, data, length);
	}, NULL);
	png_set_IHDR(png.get(), info, input.Size().X, input.Size().Y, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png.get(), info);
	png_set_filler(png.get(), 0x00, PNG_FILLER_AFTER);
	png_set_bgr(png.get());
	png_write_image(png.get(), const_cast<png_bytepp>(rowPointers.data()));
	png_write_end(png.get(), NULL);

	return std::make_unique<std::vector<char>>(std::move(output));
}

const static char hex[] = "0123456789ABCDEF";

ByteString format::URLEncode(ByteString source)
{
	ByteString result;
	for (auto it = source.begin(); it < source.end(); ++it)
	{
		if (!((*it >= 'a' && *it <= 'z') ||
		      (*it >= 'A' && *it <= 'Z') ||
		      (*it >= '0' && *it <= '9')))
		{
			auto byte = uint8_t(*it);
			result.append(1, '%');
			result.append(1, hex[(byte >> 4) & 0xF]);
			result.append(1, hex[ byte       & 0xF]);
		}
		else
		{
			result.append(1, *it);
		}
	}
	return result;
}

ByteString format::URLDecode(ByteString source)
{
	ByteString result;
	for (auto it = source.begin(); it < source.end(); ++it)
	{
		if (*it == '%' && it < source.end() + 2)
		{
			auto byte = uint8_t(0);
			for (auto i = 0; i < 2; ++i)
			{
				it += 1;
				auto *off = strchr(hex, tolower(*it));
				if (!off)
				{
					return {};
				}
				byte = (byte << 4) | (off - hex);
			}
			result.append(1, byte);
		}
		else if (*it == '+')
		{
			result.append(1, ' ');
		}
		else
		{
			result.append(1, *it);
		}
	}
	return result;
}

void format::RenderTemperature(StringBuilder &sb, float temp, int scale)
{
	switch (scale)
	{
	case 1:
		sb << (temp - 273.15f) << "C";
		break;
	case 2:
		sb << (temp - 273.15f) * 1.8f + 32.0f << "F";
		break;
	default:
		sb << temp << "K";
		break;
	}
}

float format::StringToTemperature(String str, int defaultScale)
{
	auto scale = defaultScale;
	if (str.size())
	{
		if (str.EndsWith("K"))
		{
			scale = 0;
			str = str.SubstrFromEnd(1);
		}
		else if (str.EndsWith("C"))
		{
			scale = 1;
			str = str.SubstrFromEnd(1);
		}
		else if (str.EndsWith("F"))
		{
			scale = 2;
			str = str.SubstrFromEnd(1);
		}
	}
	if (!str.size())
	{
		throw std::out_of_range("empty string");
	}
	auto out = str.ToNumber<float>();
	switch (scale)
	{
	case 1:
		out = out + 273.15;
		break;
	case 2:
		out = (out - 32.0f) / 1.8f + 273.15f;
		break;
	}
	return out;
}
