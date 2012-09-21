//Syntax analyser
#include "Parser.h"
namespace pim
{
	namespace compiler
	{
		Parser::Parser(std::stringstream & source_) :
			source(source_)
		{
			scanner = new Scanner(source);
			generator = new Generator();

			token = scanner->NextToken();

		}

		std::vector<unsigned char> Parser::Compile()
		{
			program();
			return generator->Finish();
		}

		/*
			<program> ::= <function list>
		*/
		void Parser::program()
		{
			functionList();
		}
		
		/*
			<function list> ::=	<function> | <function> <function list>
		*/
		void Parser::functionList()
		{
			function();
			while(look(Token::FunctionSymbol))
				function();
		}
		
		/*
			<function> ::= function identifier ( <declaration list> ) <block> end
		*/
		void Parser::function()
		{
			std::string functionName;

			expect(Token::FunctionSymbol);

			functionName = token.Source;
			//generator->ScopeLabel(functionName);	//Function name
			generator->PushScope(functionName);
			expect(Token::Identifier);

			expect(Token::LeftBracket);
			if(!accept(Token::RightBracket))
			{
				argumentList();
				expect(Token::RightBracket);
			}
			block();
			expect(Token::EndSymbol);
			generator->Return();

			generator->PopScope();
		}
		
		/*
			<function call>	::=	identifier ( <expression list> )
		*/
		void Parser::functionCall()
		{
			std::string functionName;

			functionName = token.Source;
			expect(Token::Identifier);
			expect(Token::LeftBracket);
			expressionList();
			expect(Token::RightBracket);
			//generator->Call(functionName);
		}
		
		/*
			<block> ::= <declaration list> <statement list>
		*/
		void Parser::block()
		{
			if(look(Token::IntegerSymbol) || look(Token::DecimalSymbol) || look(Token::ParticleSymbol))
				declarationList();
			statementList();
		}
		
		/*
			<argument list> ::= <argument> | <argument> , <argument list>
		*/
		void Parser::argumentList()
		{
			argument();
			while(accept(Token::CommaSymbol))
				argument();
		}
		
		/*
			<argument> ::= integer identifier | decimal identifier | particle identifier
		*/
		void Parser::argument()
		{
			generator->ScopeVariableType(token.Symbol);
			if(!accept(Token::IntegerSymbol))
				if(!accept(Token::DecimalSymbol))
					if(!accept(Token::ParticleSymbol))
						throw ParserExpectException(token, "type name");
			generator->ScopeVariable(token.Source);
			expect(Token::Identifier);
		}

		/*
			<declaration list> ::= <declaration> | <declaration> , <declaration list>
		*/
		void Parser::declarationList()
		{
			declaration();
			while(accept(Token::CommaSymbol))
				declaration();
		}
		
		/*
			<declaration> ::= integer <identifier list> | decimal <identifier list> | particle <identifier list>
		*/
		void Parser::declaration()
		{
			generator->ScopeVariableType(token.Symbol);
			if(!accept(Token::IntegerSymbol))
				if(!accept(Token::DecimalSymbol))
					if(!accept(Token::ParticleSymbol))
						throw ParserExpectException(token, "type name");
			identifierList();
		}
		
		/*
			<identifier list> ::= identifier | identifier , <identifier list>
		*/
		void Parser::identifierList()
		{
			generator->ScopeVariable(token.Source);
			expect(Token::Identifier);
			while(accept(Token::CommaSymbol))
			{
				generator->ScopeVariable(token.Source);
				expect(Token::Identifier);
			}
		}
		
		/*
			<statement list> ::= <statement> | <statement> <statement list>
		*/
		void Parser::statementList()
		{
			statement();
			while(!look(Token::EndSymbol))
				statement();
		}
		
		/*
			<statement> ::= <neighbour statement> | <if statement> | <assignment statement> | <function call> | <particle action> | break | continue
		*/
		void Parser::statement()
		{
			//generator->Begin(NonTerminal::Statement);
			if(look(Token::NeighbourSymbol))
			{
				neighbourStatement();
			}
			else if(look(Token::IfSymbol))
			{
				ifStatement();
			}
			else if(look(Token::CreateSymbol) || look(Token::KillSymbol)  || look(Token::GetSymbol) || look(Token::TransformSymbol))
			{
				particleAction();
				generator->Discard();
			}
			else if(look(Token::BreakSymbol))
			{
				expect(Token::BreakSymbol);
				generator->Jump(breakLabel);			
			}
			else if(look(Token::ContinueSymbol))
			{
				expect(Token::ContinueSymbol);
				generator->Jump(continueLabel);
			}
			else if(look(Token::Identifier))
			{
				assigmentStatement();
			}
			//generator->End(NonTerminal::Statement);
		}

		/*
			<particle action> ::= <kill statement> | <create statement> | <transform statement>
		*/
		void Parser::particleAction()
		{
			if(look(Token::KillSymbol))
			{
				killStatement();
			}
			else if(look(Token::CreateSymbol))
			{
				createStatement();
			}
			else if(look(Token::TransformSymbol))
			{
				transformStatement();
			}

		}
		
		/*
			<kill statement> ::= kill ( <expression> )
		*/
		void Parser::killStatement()
		{
			expect(Token::KillSymbol);
			expect(Token::LeftBracket);
			expression();
			expect(Token::RightBracket);
			generator->KillParticle();
		}
		
		/*
			<create statement> ::= create ( <expression>, <expression>, <expression>, <expression> )
		*/
		void Parser::createStatement()
		{
			expect(Token::CreateSymbol);
			expect(Token::LeftBracket);
			expression();
			expect(Token::CommaSymbol);
			expression();
			expect(Token::CommaSymbol);
			expression();
			expect(Token::CommaSymbol);
			expression();
			expect(Token::RightBracket);
			generator->CreateParticle();
		}
		
		/*
			<transform statement> ::= transform ( <expression>, <expression> )
		*/
		void Parser::transformStatement()
		{
			expect(Token::TransformSymbol);
			expect(Token::LeftBracket);
			expression();
			expect(Token::CommaSymbol);
			expression();
			expect(Token::RightBracket);
			generator->TransformParticle();
		}

		/*
			<get statement> ::= get ( <expression>, <expression> )
		*/
		void Parser::getStatement()
		{
			expect(Token::GetSymbol);
			expect(Token::LeftBracket);
			expression();
			expect(Token::CommaSymbol);
			expression();
			expect(Token::RightBracket);
			generator->GetParticle();
		}

		/*
			<neighbour statement> ::= neighbour identifier for <expression> do <block> end | neighbour identifier for <expression>, <expression> do <block> end
		*/
		void Parser::neighbourStatement()
		{
			std::string neighbourVariable;
			std::string loopLabel = generator->UniqueLabel("neighbour");
			std::string xVar = loopLabel+"X";
			std::string xMin = loopLabel+"minX";
			std::string xMax = loopLabel+"maxX";
			std::string yVar = loopLabel+"Y";
			std::string yMax = loopLabel+"maxY";
			breakLabel = loopLabel+"End";
			continueLabel = loopLabel+"Next";

			expect(Token::NeighbourSymbol);

			generator->PushLocalScope(loopLabel+"Start");
			neighbourVariable = token.Source;
			expect(Token::Identifier);
			generator->ScopeVariableType(Token::IntegerConstant);
			generator->ScopeVariable(neighbourVariable);
			generator->ScopeVariable(xVar);
			generator->ScopeVariable(yVar);
			generator->ScopeVariable(xMin);
			generator->ScopeVariable(xMax);
			generator->ScopeVariable(yMax);

			generator->LocalEnter();

			expect(Token::OfSymbol);

			//Initialise position
			expression();
			generator->GetPosition();
			generator->Duplicate();
			generator->Increment("-1");
			generator->StoreVariable(yVar);
			generator->Increment("1");
			generator->StoreVariable(yMax);

			generator->Duplicate();
			generator->Increment("-1");
			generator->Duplicate();
			generator->StoreVariable(xVar);
			generator->StoreVariable(xMin);
			generator->Increment("1");
			generator->StoreVariable(xMax);

			//if(accept(Token::CommaSymbol))
			//	expression();
			expect(Token::DoSymbol);
			
			generator->ScopeLabel(loopLabel+"Next");





			//Check X
			generator->LoadVariable(xVar);
			generator->LoadVariable(xMax);
			//generator->Duplicate();	//Duplicate xvar so it can be used for incrementing

			generator->JumpLessEqual(loopLabel+"Begin");
			//if(xVar > xMax) {

				//Reset X, increment Y
				generator->LoadVariable(xMin);
				generator->StoreVariable(xVar);

				generator->LoadVariable(yVar);
				generator->Increment("1");
				generator->Duplicate();
				generator->StoreVariable(yVar);


				//Check Y
				generator->LoadVariable(yMax);
				generator->JumpGreater(loopLabel+"End");

			//}

			//Start of loop
			generator->ScopeLabel(loopLabel+"Begin");

			generator->LoadVariable(xVar);
			generator->LoadVariable(yVar);
			generator->GetParticle();
			generator->StoreVariable(neighbourVariable);

			block();
			
			//Increment X
			generator->LoadVariable(xVar);
			generator->Increment("1");
			generator->StoreVariable(xVar);

			//Next element
			generator->Jump(loopLabel+"Next");
			
			generator->ScopeLabel(loopLabel+"End");
			generator->Return();
			generator->PopScope();
			expect(Token::EndSymbol);
		}
		
		/*
			<if statement> ::= if <condition> then <block> end
		*/
		void Parser::ifStatement()
		{
			//generator->Begin(NonTerminal::IfStatement);
			expect(Token::IfSymbol);
			condition();
			expect(Token::ThenSymbol);
			block();
			expect(Token::EndSymbol);
			//generator->End(NonTerminal::IfStatement);
		}
		
		/*
			<condition> ::= identifier <conditional operator> identifier | identifier <conditional operator> numberConstant	| numberConstant <conditional operator> identifier | numberConstant <conditional operator> numberConstant
		*/
		void Parser::condition()
		{
			//generator->Begin(NonTerminal::Condition);
			if(look(Token::Identifier))
			{
				conditionalOperator();
				if(!accept(Token::Identifier) && !accept(Token::IntegerConstant) && !accept(Token::DecimalConstant))
					throw ParserExpectException(token, "identifier or constant");
			}
			else if(look(Token::DecimalConstant) || look(Token::IntegerConstant))
			{
				conditionalOperator();
				if(!accept(Token::Identifier) && !accept(Token::IntegerConstant) && !accept(Token::DecimalConstant))
					throw ParserExpectException(token, "identifier or constant");	
			}
			else
			{
				throw ParserExpectException(token, "condition");
			}
			//generator->End(NonTerminal::Condition);
		}

		/*
			<conditional operator> ::= > | >= | == | != | < | <=
		*/
		void Parser::conditionalOperator()
		{
			//generator->Begin(NonTerminal::ConditionalOperator);
			if(!accept(Token::GreaterSymbol))
				if(!accept(Token::GreaterEqualSymbol))
					if(!accept(Token::EqualSymbol))
						if(!accept(Token::NotEqualSymbol))
							if(!accept(Token::LessSymbol))
								if(!accept(Token::LessEqualSymbol))
									throw ParserExpectException(token, "conditional operator");
			//generator->End(NonTerminal::ConditionalOperator);
		}
		
		/*
			<assigment statement> ::= identifier = <expression> | identifier.property = <expression>
		*/
		void Parser::assigmentStatement()
		{
			std::string variable = token.Source;
			expect(Token::Identifier);
			if(accept(Token::AssignSymbol))
			{
				expression();
				generator->StoreVariable(variable);
			}
			else if(accept(Token::DotSymbol))
			{
				std::string property = token.Source;
				expect(Token::Identifier);
				expect(Token::AssignSymbol);
				expression();
				generator->LoadVariable(variable);
				generator->StoreProperty(property);	
			}
		}
		
		/*
			<expression list> ::= <expression> | <expression> , <expression list>
		*/
		void Parser::expressionList()
		{
			//generator->Begin(NonTerminal::ExpressionList);
			expression();
			while(accept(Token::CommaSymbol))
				expression();
			//generator->End(NonTerminal::ExpressionList);
		}
		
		/*
			<expression> ::= <term> | <expression> + <term> | <expression> - <term>
		*/
		void Parser::expression()
		{
			term();
			int as = token.Symbol;
			while(accept(Token::PlusSymbol) || accept(Token::MinusSymbol))
			{
				term();
				if(as == Token::PlusSymbol)
					generator->Add();
				else if(as == Token::MinusSymbol)
					generator->Subtract();
			}
			//generator->End(NonTerminal::Expression);
		}
		
		/*
			<term> ::= <factor> | <term> * <factor> | <term> / <factor>
		*/
		void Parser::term()
		{
			//generator->Begin(NonTerminal::Term);
			factor();
			int md = token.Symbol;
			while(accept(Token::MultiplySymbol) || accept(Token::DivideSymbol))
			{
				factor();
				if(md == Token::MultiplySymbol)
					generator->Multiply();
				else if(md == Token::DivideSymbol)
					generator->Divide();
			}
			//generator->End(NonTerminal::Term);
		}
		
		/*
			<factor> ::= <variable value> | - <variable value> | numberConstant | - numberConstant | ( <expression> ) | - ( <expression> )
		*/
		void Parser::factor()
		{
			bool doNegate = false;
			std::string factor = token.Source;
			if(accept(Token::MinusSymbol))
			{
				factor = token.Source;
				doNegate = true;
			}
			if(accept(Token::IntegerConstant) || accept(Token::DecimalConstant))
			{
				if(doNegate)
				{
					doNegate = false;
					generator->Constant("-" + factor);
				}
				else
					generator->Constant(factor);
			}
			else if(accept(Token::LeftBracket))
			{
				expression();
				expect(Token::RightBracket);
			}
			else
			{
				variableValue();
			}
			if(doNegate)
				generator->Negate();
		}

		/*
			<variable value> ::= <function call> | identifier | identifier.property | <particle action>
		*/
		void Parser::variableValue()
		{
			std::string variable = token.Source;
			if(accept(Token::Identifier))
			{
				if(look(Token::LeftBracket))
				{
					back();
					functionCall();
				}
				else
				{
					if(accept(Token::DotSymbol))
					{
						std::string property = token.Source;
						expect(Token::Identifier);
						generator->LoadVariable(variable);
						generator->LoadProperty(property);
					}
					else
					{
						generator->LoadVariable(variable);
					}
				}
			}
			else
			{
				particleAction();
			}
		}

		bool Parser::accept(int symbol)
		{
			if(symbol == token.Symbol)
			{
				//generator->Insert(token);
				lastToken = token;
				if(previousTokens.size())
				{
					token = previousTokens.top();
					previousTokens.pop();
				}
				else
					token = scanner->NextToken();
				std::cout << "Symbol " << Token::SymbolNames[symbol] << " " << lastToken.Source << std::endl;
				return true;
			}
			std::cout << "Bad Symbol " << Token::SymbolNames[symbol] << " " << token.Source << " (" << token.GetName() << ")" << std::endl;
			return false;
		}


		bool Parser::look(int symbol)
		{
			if(symbol == token.Symbol)
				return true;
			return false;
		}

		void Parser::back()
		{
			previousTokens.push(token);
			token = lastToken;
		}

		void Parser::expect(int symbol)
		{
			if(!accept(symbol))
				throw ParserExpectException(token, symbol);
		}
	}
}