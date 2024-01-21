#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

static void writeU32LE(uint8_t *dest, uint32_t value)
{
	dest[0] = uint8_t( value        & 0xFF);
	dest[1] = uint8_t((value >>  8) & 0xFF);
	dest[2] = uint8_t((value >> 16) & 0xFF);
	dest[3] = uint8_t((value >> 24) & 0xFF);
}

static uint32_t readU32BE(const uint8_t *src)
{
	return  uint32_t(src[3])        |
	       (uint32_t(src[2]) <<  8) |
	       (uint32_t(src[1]) << 16) |
	       (uint32_t(src[0]) << 24);
}

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		std::cerr << "usage: " << argv[0] << " OUTPUT INPUT..." << std::endl;
		exit(1);
	}
	auto images = argc - 2;
	if (images > 255)
	{
		std::cerr << "too many images specified" << std::endl;
		exit(1);
	}
	std::string outputPath = argv[1];
	std::ofstream output(outputPath, std::ios::binary);
	auto outputFailure = [&outputPath](std::string action) {
		std::cerr << "failed to " << action << " " << outputPath << ": " << strerror(errno) << std::endl;
		exit(1);
	};
	if (!output)
	{
		outputFailure("open");
	}
	std::vector<char> header(6 + images * 16, 0);
	auto writeHeader = [&header, &output, &outputFailure]() {
		output.seekp(0, std::ios_base::beg);
		output.write(&header[0], header.size());
		if (!output)
		{
			outputFailure("write");
		}
	};
	writeHeader(); // make space for header
	auto *headerU8 = reinterpret_cast<uint8_t *>(&header[0]);
	headerU8[2] = 1;
	headerU8[4] = images;
	for (auto image = 0; image < images; ++image)
	{
		std::string inputPath = argv[2 + image];
		std::ifstream input(inputPath, std::ios::binary);
		auto inputFailure = [&inputPath](std::string action) {
			std::cerr << "failed to " << action << " " << inputPath << ": " << strerror(errno) << std::endl;
			exit(1);
		};
		auto imageFailure = [&inputPath](std::string failure) {
			std::cerr << "failed to process " << inputPath << ": " << failure << std::endl;
			exit(1);
		};
		if (!input)
		{
			inputFailure("open");
		}
		std::vector<char> buf;
		input.seekg(0, std::ios_base::end);
		buf.resize(input.tellg());
		input.seekg(0, std::ios_base::beg);
		input.read(&buf[0], buf.size());
		if (!input)
		{
			inputFailure("read");
		}
		auto *bufU8 = reinterpret_cast<uint8_t *>(&buf[0]);
		if (buf.size() < 0x21 ||
			readU32BE(&bufU8[0]) != UINT32_C(0x89504E47) ||
			readU32BE(&bufU8[4]) != UINT32_C(0x0D0A1A0A) ||
		    bufU8[0x18] != 8 ||
		    bufU8[0x19] != 6)
		{
			imageFailure("not a 32bpp RGBA PNG");
		}
		auto writeOffset = output.tellp();
		output.write(&buf[0], buf.size());
		if (!output)
		{
			outputFailure("write");
		}
		auto width = readU32BE(&bufU8[0x10]);
		auto height = readU32BE(&bufU8[0x14]);
		if (width == 256)
		{
			width = 0;
		}
		if (width > 255)
		{
			imageFailure("width exceeds U8 limit");
		}
		if (height == 256)
		{
			height = 0;
		}
		if (height > 255)
		{
			imageFailure("height exceeds U8 limit");
		}
		auto *entryU8 = headerU8 + 6 + image * 16;
		entryU8[0] = width;
		entryU8[1] = height;
		entryU8[4] = 1;
		entryU8[6] = 32;
		if (buf.size() > UINT32_MAX)
		{
			imageFailure("data size exceeds U32 limit");
		}
		writeU32LE(&entryU8[8], uint32_t(buf.size()));
		if (writeOffset > UINT32_MAX)
		{
			std::cerr << "output data size exceeds U32 limit" << std::endl;
			exit(1);
		}
		writeU32LE(&entryU8[12], uint32_t(writeOffset));
	}
	writeHeader(); // actually write it out
	return 0;
}
