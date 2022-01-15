#include <fstream>

int main(int argc, char *argv[])
{
	if (argc != 5)
	{
		return 1;
	}
	auto *outputCppPath = argv[1];
	auto *outputHPath = argv[2];
	auto *inputAnyPath = argv[3];
	auto *symbolName = argv[4];
	std::ifstream inputAny(inputAnyPath, std::ios::binary);
	std::ofstream outputCpp(outputCppPath);
	if (!outputCpp)
	{
		return 2;
	}
	outputCpp << "#include \"" << outputHPath << "\"\nconst unsigned char " << symbolName << "[] = { ";
	auto dataLen = 0U;
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
		outputCpp << (unsigned int)(unsigned char)(ch) << ", ";
		dataLen += 1;
	}
	outputCpp << " }; const unsigned int " << symbolName << "_size = " << dataLen << ";\n";
	if (!outputCpp)
	{
		return 4;
	}
	std::ofstream outputH(outputHPath);
	if (!outputH)
	{
		return 5;
	}
	outputH << "#pragma once\nextern const unsigned char " << symbolName << "[]; extern const unsigned int " << symbolName << "_size;\n";
	if (!outputH)
	{
		return 6;
	}
	return 0;
}
