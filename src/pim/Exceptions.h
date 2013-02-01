#pragma once

#include <cstring>
#include <string>
#include "Token.h"

namespace pim
{
	namespace compiler {
		class ParserExpectException: public std::exception
		{
			char * error;
		public:
			ParserExpectException(Token token, int expectingSymbol) {
				error = strdup(std::string("Expecting " + Token::SymbolNames[expectingSymbol] + " got " + token.Source).c_str());
			}
			ParserExpectException(Token token, std::string expectingString) {
				error = strdup(std::string("Expecting " + expectingString + " got " + token.Source).c_str());
			}
			const char * what() const throw()
			{
				return error;
			}
			~ParserExpectException() throw() {};
		};
		class TypeException: public std::exception
		{
			char * error;
		public:
			TypeException(int type, int expectingType) {
				error = strdup(std::string("Expecting a particular type, got a different type").c_str());
			}
			const char * what() const throw()
			{
				return error;
			}
			~TypeException() throw() {};
		};
	}
}