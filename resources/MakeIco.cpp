#include <fstream>
#include <vector>
#include <cstdint>

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		return 1;
	}
	auto *outputIcoPath = argv[1];
	std::ofstream outputIco(outputIcoPath, std::ios::binary);
	if (!outputIco)
	{
		return 2;
	}
	auto images = argc - 2;
	std::vector<char> header(22 + images * 16);
	auto *incondir = &header[0];
	*reinterpret_cast<uint16_t *>(&incondir[0]) = 0; // reserved
	*reinterpret_cast<uint16_t *>(&incondir[2]) = 1; // icon
	*reinterpret_cast<uint16_t *>(&incondir[4]) = uint16_t(images);
	std::vector<char> allData;
	for (auto i = 0; i < images; ++i)
	{
		auto *inputAnyPath = argv[i + 2];
		std::ifstream inputAny(inputAnyPath, std::ios::binary);
		std::vector<char> data;
		while (true)
		{
			char ch;
			inputAny.read(&ch, 1);
			if (inputAny.eof())
			{
				break;
			}
			if (!inputAny)
			{
				return 3;
			}
			data.push_back(ch);
		}
		if (*reinterpret_cast<uint64_t *>(&data[0]) != UINT64_C(0x0A1A0A0D474E5089)) // png magic
		{
			return 5;
		}
		auto width = uint8_t(data[19]);
		auto height = uint8_t(data[23]);
		auto *incondirentry = &header[6 + i * 16];
		*reinterpret_cast<uint8_t *>(&incondirentry[0]) = width;
		*reinterpret_cast<uint8_t *>(&incondirentry[1]) = height;
		*reinterpret_cast<uint8_t *>(&incondirentry[2]) = 0; // no color palette
		*reinterpret_cast<uint8_t *>(&incondirentry[3]) = 0; // reserved
		*reinterpret_cast<uint16_t *>(&incondirentry[4]) = 1; // 1 color plane
		*reinterpret_cast<uint16_t *>(&incondirentry[6]) = 32; // 32 bits per pixel
		*reinterpret_cast<uint32_t *>(&incondirentry[8]) = uint32_t(data.size()); // data size
		*reinterpret_cast<uint32_t *>(&incondirentry[12]) = uint32_t(header.size() + allData.size()); // data offset
		allData.insert(allData.end(), data.begin(), data.end());
	}
	outputIco.write(&header[0], header.size());
	outputIco.write(&allData[0], allData.size());
	if (!outputIco)
	{
		return 4;
	}
	return 0;
}
