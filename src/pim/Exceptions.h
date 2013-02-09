#pragma once

#include <cstring>
#include <string>
#include <cstdio>
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
		class ScannerCharacterException: public std::exception
		{
			char * error;
		public:
			ScannerCharacterException(char character, int lineNumber, int characterNumber) {
				error = new char[256];
				sprintf(error, "Unexpected character '%c' at line %d, char %d", character, lineNumber, characterNumber);
			}
			const char * what() const throw()
			{
				return error;
			}
			~ScannerCharacterException() throw() { delete[] error; };
		};
	}
	class FunctionNotFoundException: public std::exception
	{
		char * error;
	public:
		FunctionNotFoundException(std::string functionName, std::string returnType, std::string arguments)
		{
			error = new char[functionName.length() + returnType.length() + arguments.length()];
			sprintf(error, "Could not find function \"%s\" of type %s(%s)", functionName.c_str(), returnType.c_str(), arguments.c_str());
		}
		const char * what() const throw()
		{
			return error;
		}
		~FunctionNotFoundException() throw() { delete[] error; };
	};
}