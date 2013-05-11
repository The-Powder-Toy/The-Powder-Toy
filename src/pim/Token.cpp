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
			"else",
			"elseif",
			"then",
			"end",
			"kill",
			"create",
			"transform",
			"get",
			"RUNTIMEMACRO",
			"IDENTIFIER",
			",",
			".",
			"INVALID SYMBOL"
		};
	}
}
