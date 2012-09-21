#include "Token.h"

namespace pim
{
	namespace compiler
	{
		std::string Token::SymbolNames[] = {
			"=",
			"function",
			"(",
			")",
			"/",
			"*",
			"+",
			"-",
			"%",
			"INTEGER",
			"DECIMAL",
			"PARTICLE",
			"integer",
			"decimal",
			"particle",
			"is",
			"<",
			"<=",
			">",
			">=",
			"==",
			"!=",
			"neighbour",
			"do",
			"of",
			"break",
			"continue",
			"if",
			"then",
			"end",
			"kill",
			"create",
			"transform",
			"get",
			"IDENTIFIER",
			",",
			".",
			"INVALID SYMBOL"
		};
	}
}