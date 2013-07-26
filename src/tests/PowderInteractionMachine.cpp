#ifdef TEST

#include <iostream>
#include <fstream>
#include <sstream>
#include "pim/Parser.h"

int main(int argc, char * argv[])
{
	std::ifstream file("test.p");

	std::stringstream buffer;

	buffer << file.rdbuf();
	file.close();

	pim::compiler::Parser * parser = new pim::compiler::Parser(buffer);

	parser->Compile();
}
#endif
