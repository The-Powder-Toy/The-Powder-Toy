//Lexical analyser
#include <algorithm>
#include <cctype>
#include "Scanner.h"

namespace pim
{
	namespace compiler
	{
		Scanner::Scanner(std::stringstream & source_) :
			source(source_)
		{
			nextCharacter();
		}

		Token Scanner::NextToken()
		{
			//Read whitespace, newlines and comments
			while(
				cChar == ' ' ||	cChar == '\t' ||
				cChar == '\r' || cChar == '\n' ||
				cChar == '/')
			{
				if(cChar == '/')
				{
					nextCharacter();
					if(cChar == '/')
					{
						while(cChar != '\n' && cChar != '\r')
							nextCharacter();
					}
					else
						return Token(Token::DivideSymbol, "/", cLine);
				}

				if(cChar == '\r')
				{
					nextCharacter();
					if(cChar == '\n')
						cLine++;
					else
						continue;
				}
				else if(cChar == '\n')
					cLine++;

				nextCharacter();
			}

			if(std::isalpha(cChar))	//Read alphanumeric symbols
			{
				cToken.clear();
				while(std::isalpha(cChar) || std::isdigit(cChar))
				{
					cToken.push_back(cChar);
					nextCharacter();
				}

				std::transform(cToken.begin(), cToken.end(), cToken.begin(), ::tolower);

				for(int i = 0; i < Token::SymbolNumber; i++)
					if(Token::SymbolNames[i] == cToken)
						return Token(i, cToken, cLine);
				return Token(Token::Identifier, cToken, cLine);
			}
			else if(std::isdigit(cChar)) //Read numeric constants
			{
				bool decimal = false;
				cToken.clear();
				while(std::isdigit(cChar))
				{
					cToken.push_back(cChar);
					nextCharacter();
				}
				if(cChar == '.')
				{
					decimal = true;
					cToken.push_back(cChar);
					nextCharacter();
					while(std::isdigit(cChar))
					{
						cToken.push_back(cChar);
						nextCharacter();
					}
				}
				if(decimal)
					return Token(Token::DecimalConstant, cToken, cLine);
				return Token(Token::IntegerConstant, cToken, cLine);
			}
			else if(cChar == '[')
			{
				cToken.clear();
				nextCharacter();
				while(std::isalpha(cChar) || std::isdigit(cChar) || cChar == '_' || cChar == '-')
				{
					cToken.push_back(cChar);
					nextCharacter();
				}
				nextCharacter();

				std::transform(cToken.begin(), cToken.end(), cToken.begin(), ::toupper);

				return Token(Token::RTMacro, cToken, cLine);
			}
			else if(cChar == '=')
			{
				nextCharacter();
				if(cChar == '=')
				{
					nextCharacter();
					return Token(Token::EqualSymbol, "==", cLine);
				}
				return Token(Token::AssignSymbol, "=", cLine);
			}
			else if(cChar == '!')
			{
				nextCharacter();
				if(cChar == '=')
					return Token(Token::NotEqualSymbol, "==", cLine);
			}
			else if(cChar == '(')
			{
				nextCharacter();
				return Token(Token::LeftBracket, "(", cLine);
			}
			else if(cChar == ')')
			{
				nextCharacter();
				return Token(Token::RightBracket, ")", cLine);
			}
			else if(cChar == '/')
			{
				nextCharacter();
				return Token(Token::DivideSymbol, "/", cLine);
			}
			else if(cChar == '*')
			{
				nextCharacter();
				return Token(Token::MultiplySymbol, "*", cLine);
			}
			else if(cChar == '+')
			{
				nextCharacter();
				return Token(Token::PlusSymbol, "+", cLine);
			}
			else if(cChar == '-')
			{
				nextCharacter();
				return Token(Token::MinusSymbol, "-", cLine);
			}
			else if(cChar == '%')
			{
				nextCharacter();
				return Token(Token::ModuloSymbol, "%", cLine);
			}
			else if(cChar == '<')
			{
				nextCharacter();
				if(cChar == '=')
				{
					return Token(Token::LessEqualSymbol, "<=", cLine);
				}
				return Token(Token::LessSymbol, "<", cLine);
			}
			else if(cChar == '>')
			{
				nextCharacter();
				if(cChar == '=')
				{
					return Token(Token::GreaterEqualSymbol, ">=", cLine);
				}
				return Token(Token::GreaterSymbol, ">", cLine);
			}
			else if(cChar == ',')
			{
				nextCharacter();
				return Token(Token::CommaSymbol, ",", cLine);
			}
			else if(cChar == '.')
			{
				nextCharacter();
				return Token(Token::DotSymbol, ".", cLine);
			}
			else
			{
				nextCharacter();
				return Token(Token::InvalidSymbol, std::string(1, cChar), cLine);
			}
		}

		void Scanner::nextCharacter()
		{
			if(source.good())
				cChar = source.get();
			else
				cChar = 0;
		}
	}
}
