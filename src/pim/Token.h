#pragma once

#include <string>
namespace pim
{
	namespace compiler
	{
		class Token
		{
		public:
			static std::string SymbolNames[];

			enum
			{
				AssignSymbol = 0,
				FunctionSymbol,

				LeftBracket,
				RightBracket,
				DivideSymbol,
				MultiplySymbol,
				PlusSymbol,
				MinusSymbol,
				ModuloSymbol,

				IntegerConstant,
				DecimalConstant,
				ParticleConstant,

				IntegerSymbol,
				DecimalSymbol,
				ParticleSymbol,

				IsSymbol,
				LessSymbol,
				LessEqualSymbol,
				GreaterSymbol,
				GreaterEqualSymbol,
				NotEqualSymbol,
				EqualSymbol,

				NeighbourSymbol,
				DoSymbol,
				OfSymbol,
				BreakSymbol,
				ContinueSymbol,
				IfSymbol,
				ElseSymbol,
				ElseIfSymbol,
				ThenSymbol,
				EndSymbol,

				KillSymbol,
				CreateSymbol,
				TransformSymbol,
				GetSymbol,

				RTMacro,
				Identifier,

				CommaSymbol,
				DotSymbol,

				InvalidSymbol,

				SymbolNumber
			};
			int Symbol;
			int LineNumber;
			std::string Source;

			Token(int symbol = InvalidSymbol, std::string source = "HERP DERP", int lineNumber = 0) :
				Symbol(symbol),
				Source(source),
				LineNumber(lineNumber) {}

			std::string GetName()
			{
				return SymbolNames[Symbol];
			}
		};
	}
}
