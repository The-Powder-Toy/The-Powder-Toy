#pragma once

#include <string>
#include <sstream>
#include "Token.h"
namespace pim
{
	namespace compiler
	{
		class Scanner
		{
			char cChar;
			int cLine;
			std::string cToken;
			std::stringstream & source;
			void nextCharacter();
		public:
			Scanner(std::stringstream & source_);
			Token NextToken();
		};
	}
}
