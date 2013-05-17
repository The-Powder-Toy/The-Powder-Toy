#pragma once

#include <string>
#include <cstring>
#include <sstream>
#include "Scanner.h"
#include "Generator.h"
#include "Token.h"
namespace pim
{
	namespace compiler
	{
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
		class Parser
		{
			std::stringstream & source;
			Generator * generator;
			Scanner * scanner;
			Token token;
			Token lastToken;
			std::string breakLabel;
			std::string continueLabel;
			std::stack<Token> previousTokens;

			void program();
			void functionList();
			void function();
			void functionCall();
			void block();
			void argumentList();
			void argument();
			void declarationList();
			void declaration();
			void identifierList();
			void statementList();
			void statement();
			void neighbourStatement();
			void ifStatement();
			void condition(std::string jumpLabel);
			void assigmentStatement();
			void particleAction();
			void killStatement();
			void getStatement();
			void createStatement();
			void transformStatement();
			void expressionList();

			void expression();
			void term();
			void factor();
			void variableValue();

			Token forward();
			bool accept(int symbol);
			bool look(int symbol);
			void back();
			void expect(int symbol);
		public:
			Parser(std::stringstream & source_);

			std::vector<unsigned char> Compile();
		};
	}
}
