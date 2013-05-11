#pragma once

#include <cstring>
#include <vector>
#include <stack>
#include <iostream>
#include "Token.h"
namespace pim
{
	namespace compiler
	{
		class VariableNotFoundException: public std::exception
		{
			char * error;
		public:
			VariableNotFoundException(std::string variable) {
				error = strdup(std::string("Could not find the variable \""+variable+"\" in the current scope").c_str());
			}
			const char * what() const throw()
			{
				return error;
			}
			~VariableNotFoundException() throw() {};
		};

		class SymbolNotFoundException: public std::exception
		{
			char * error;
		public:
			SymbolNotFoundException(std::string variable) {
				error = strdup(std::string("Could not find the symbol \""+variable+"\".").c_str());
			}
			const char * what() const throw()
			{
				return error;
			}
			~SymbolNotFoundException() throw() {};
		};
		class Type
		{
			enum { Integer = Token::IntegerSymbol, Decimal = Token::DecimalSymbol };
		};
		class Definition
		{
		public:
			std::string Name;
			int Type;
			int StackPosition;
			Definition(std::string name, int type, int position) :
				Type(type),
				Name(name),
				StackPosition(position)
			{

			}
		};

		struct Label
		{
			std::string Name;
			int Position;
		};

		class Scope
		{
		public:
			std::vector<Definition> Definitions;
			std::vector<Label> Labels;
			int FrameSize;
			int LocalFrameSize;
			Scope():
				FrameSize(0),
				LocalFrameSize(0)
			{

			}
			Definition GetDefinition(std::string name)
			{
				for(std::vector<Definition>::iterator iter = Definitions.begin(), end = Definitions.end(); iter != end; ++iter)
				{
					if((*iter).Name == name)
						return *iter;
				}
				throw VariableNotFoundException(name);
			}
		};

		class Generator
		{
			int variableType;
			std::stack<Scope*> scopes;
			Scope * currentScope;
			std::ostream & output;
			int labelCounter;
			int programCounter;

			typedef std::pair<int, std::string> Placeholder;
			std::vector<Placeholder> placeholders;

			typedef std::pair<int, int*> ValuePlaceholder;
			std::vector<ValuePlaceholder> valuePlaceholders;

			typedef std::pair<int, std::string> PropertyPlaceholder;
			std::vector<PropertyPlaceholder> propertyPlaceholders;

			typedef std::pair<int, std::string> MacroPlaceholder;
			std::vector<MacroPlaceholder> macroPlaceholders;

			std::vector<Label> labelPositions;

			std::vector<unsigned char> program;

			void defineLabel(std::string label);
			void writeOpcode(int opcode);
			void writeConstant(std::string constant);
			void writeConstant(int constant);
			void writeConstantPlaceholder(std::string label);
			void writeConstantPlaceholder(int * value);
			void writeConstantMacroPlaceholder(std::string macro);
			void writeConstantPropertyPlaceholder(std::string property);
		
		public:
			Generator();

			std::vector<unsigned char> Finish();

			std::string UniqueLabel(std::string prefix);

			void PushScope(std::string label);
			void PushLocalScope(std::string label);
			void LocalEnter();
			void PopScope();

			void ScopeLabel(std::string label);
			void ScopeVariableType(int type);
			void ScopeVariable(std::string label);

			void PushVariableAddress(std::string label);
//			void Store();
			void LoadVariable(std::string label);
			void StoreVariable(std::string label);

			void Duplicate();
			void Discard();
			void RTConstant(std::string name);
			void Constant(std::string constant);
			void Increment(std::string constant);
			void Add();
			void Subtract();
			void Multiply();
			void Divide();
			void Modulus();
			void Negate();

			void TransformParticle();
			void CreateParticle();
			void GetParticle();
			void GetPosition();
			void KillParticle();
			void LoadProperty(std::string property);
			void StoreProperty(std::string property);

			void IntegerToDecimal();
			void DecimalToInteger();

			void JumpEqual(std::string label);
			void JumpNotEqual(std::string label);
			void JumpGreater(std::string label);
			void JumpGreaterEqual(std::string label);
			void JumpLess(std::string label);
			void JumpLessEqual(std::string label);
			void Jump(std::string label);

			void Call(int arguments, std::string label);
			void Return();
		};
	}
}
