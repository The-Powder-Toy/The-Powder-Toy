//Code generator for bytecode
#include <sstream>
#include <fstream>
#include "Format.h"
#include "Generator.h"
#include "Opcodes.h"
namespace pim
{
	namespace compiler
	{
		Generator::Generator() :
			output(std::cout),
			labelCounter(0),
			programCounter(0)
		{

		}

		void Generator::defineLabel(std::string label)
		{
			Label newLabel;
			newLabel.Name = label;
			newLabel.Position = programCounter;//program.size();
			labelPositions.push_back(newLabel);
		}

		void Generator::writeOpcode(int opcode)
		{
			programCounter++;
			program.push_back(opcode);
		}

		void Generator::writeConstant(std::string constant)
		{
			writeConstant(format::StringToNumber<int>(constant));
		}

		void Generator::writeConstant(int constant)
		{
			program.push_back(constant & 0xFF);
			program.push_back((constant>>8) & 0xFF);
			program.push_back((constant>>16) & 0xFF);
			program.push_back((constant>>24) & 0xFF);
		}

		void Generator::writeConstantPlaceholder(std::string label)
		{
			placeholders.push_back(Placeholder(program.size(), label));
			program.push_back(0);
			program.push_back(0);
			program.push_back(0);
			program.push_back(0);
		}

		void Generator::writeConstantPlaceholder(int * value)
		{
			valuePlaceholders.push_back(ValuePlaceholder(program.size(), value));
			program.push_back(0);
			program.push_back(0);
			program.push_back(0);
			program.push_back(0);
		}

		void Generator::writeConstantPropertyPlaceholder(std::string property)
		{
			propertyPlaceholders.push_back(PropertyPlaceholder(program.size(), property));
			program.push_back(0);
			program.push_back(0);
			program.push_back(0);
			program.push_back(0);
		}

		void Generator::writeConstantMacroPlaceholder(std::string macro)
		{
			macroPlaceholders.push_back(MacroPlaceholder(program.size(), macro));
			program.push_back(0);
			program.push_back(0);
			program.push_back(0);
			program.push_back(0);
		}

		std::vector<unsigned char> Generator::Finish()
		{
			//All compile time labels, macros, etc
			for(std::vector<Placeholder>::iterator iter = placeholders.begin(), end = placeholders.end(); iter != end; ++iter)
			{
				bool found = false;
				Placeholder cPosition = *iter;
				for(std::vector<Label>::iterator iter2 = labelPositions.begin(), end2 = labelPositions.end(); iter2 != end2; ++iter2)
				{
					Label cLabel = *iter2;
					if(cPosition.second == cLabel.Name)
					{
						std::cout << "Setting placeholder at " << cPosition.first << " with " << cLabel.Position << " for" << cPosition.second << std::endl;
						found = true;
						program[cPosition.first] = cLabel.Position & 0xFF;
						program[cPosition.first+1] = (cLabel.Position >> 8) & 0xFF;
						program[cPosition.first+2] = (cLabel.Position >> 16) & 0xFF;
						program[cPosition.first+3] = (cLabel.Position >> 24) & 0xFF;
						break;
					}
				}				
				if(!found)
					throw SymbolNotFoundException(cPosition.second);
			}

			for(std::vector<ValuePlaceholder>::iterator iter = valuePlaceholders.begin(), end = valuePlaceholders.end(); iter != end; ++iter)
			{
				ValuePlaceholder cPosition = *iter;
				int value = *cPosition.second;

				std::cout << "Setting value placeholder at " << cPosition.first << " with " << value << std::endl;
						

				program[cPosition.first] = value & 0xFF;
				program[cPosition.first+1] = (value >> 8) & 0xFF;
				program[cPosition.first+2] = (value >> 16) & 0xFF;
				program[cPosition.first+3] = (value >> 24) & 0xFF;
			}

			//Build file
			int macroSizePos, propSizePos, codeSizePos, macroSize = 0, propSize = 0, codeSize = program.size();
			std::vector<unsigned char> file;
			file.push_back('P');
			file.push_back('V');
			file.push_back('M');
			file.push_back('1');


			macroSizePos = file.size();
			file.push_back(0);
			file.push_back(0);
			file.push_back(0);
			file.push_back(0);

			propSizePos = file.size();
			file.push_back(0);
			file.push_back(0);
			file.push_back(0);
			file.push_back(0);

			codeSizePos = file.size();
			file.push_back(0);
			file.push_back(0);
			file.push_back(0);
			file.push_back(0);

			//Macros
			for(std::vector<MacroPlaceholder>::iterator iter = macroPlaceholders.begin(), end = macroPlaceholders.end(); iter != end; ++iter)
			{
				MacroPlaceholder cPosition = *iter;
				int position = cPosition.first;

				file.push_back(position & 0xFF);
				file.push_back((position >> 8) & 0xFF);
				file.push_back((position >> 16) & 0xFF);
				file.push_back((position >> 24) & 0xFF);
				macroSize += 4;

				file.push_back(cPosition.second.length());
				macroSize += 1;
				file.insert(file.end(), cPosition.second.begin(), cPosition.second.end());
				macroSize += cPosition.second.length();
			}

			file[macroSizePos] = macroSize & 0xFF;
			file[macroSizePos+1] = (macroSize >> 8) & 0xFF;
			file[macroSizePos+2] = (macroSize >> 16) & 0xFF;
			file[macroSizePos+3] = (macroSize >> 24) & 0xFF;


			//Macros
			for(std::vector<PropertyPlaceholder>::iterator iter = propertyPlaceholders.begin(), end = propertyPlaceholders.end(); iter != end; ++iter)
			{
				PropertyPlaceholder cPosition = *iter;
				int position = cPosition.first;

				file.push_back(position & 0xFF);
				file.push_back((position >> 8) & 0xFF);
				file.push_back((position >> 16) & 0xFF);
				file.push_back((position >> 24) & 0xFF);
				propSize += 4;

				file.push_back(cPosition.second.length());
				propSize += 1;
				file.insert(file.end(), cPosition.second.begin(), cPosition.second.end());
				propSize += cPosition.second.length();
			}

			file[propSizePos] = propSize & 0xFF;
			file[propSizePos+1] = (propSize >> 8) & 0xFF;
			file[propSizePos+2] = (propSize >> 16) & 0xFF;
			file[propSizePos+3] = (propSize >> 24) & 0xFF;

			file.insert(file.end(), program.begin(), program.end());

			file[codeSizePos] = codeSize & 0xFF;
			file[codeSizePos+1] = (codeSize >> 8) & 0xFF;
			file[codeSizePos+2] = (codeSize >> 16) & 0xFF;
			file[codeSizePos+3] = (codeSize >> 24) & 0xFF;

			std::ofstream newFile("test.pvm");
			for(std::vector<unsigned char>::iterator iter = file.begin(), end = file.end(); iter != end; ++iter)
			{
				newFile.put(*iter);
			}
			newFile.close();

			return file;
		}

		std::string Generator::UniqueLabel(std::string prefix)
		{
			std::stringstream label;
			label << prefix;
			label << "_";
			label << labelCounter;
			label << "_";
			return label.str();
		}

		void Generator::PushScope(std::string label)
		{
			scopes.push(currentScope);
			Scope * prevScope = currentScope;
			currentScope = new Scope();
			defineLabel(label);
		}

		void Generator::PushLocalScope(std::string label)
		{
			scopes.push(currentScope);
			Scope * prevScope = currentScope;
			currentScope = new Scope();
			currentScope->Definitions.insert(currentScope->Definitions.begin(), prevScope->Definitions.begin(), prevScope->Definitions.end());
			currentScope->FrameSize = prevScope->FrameSize;
			defineLabel(label);
		}

		void Generator::PopScope()
		{

			writeOpcode(Opcode::Return);
			writeConstant(currentScope->LocalFrameSize);
			currentScope = scopes.top();
			scopes.pop();
		}

		void Generator::ScopeLabel(std::string label)
		{
			//defineLabelwriteOpcode("." << label);
			defineLabel(label);
		}	

		void Generator::LocalEnter()
		{
			writeOpcode(Opcode::LocalEnter);
			writeConstantPlaceholder(&(currentScope->LocalFrameSize));
		}

		void Generator::ScopeVariableType(int type)
		{
			variableType = type;
		}

		void Generator::ScopeVariable(std::string label)
		{
			currentScope->Definitions.push_back(Definition(label, variableType, currentScope->FrameSize));
			currentScope->FrameSize += 4;
			currentScope->LocalFrameSize += 4;
		}

		void Generator::PushVariableAddress(std::string label)
		{
			//writeOpcode("address");  << " " << currentScope->GetDefinition(label).StackPosition 
		}

		void Generator::LoadVariable(std::string label)
		{
			writeOpcode(Opcode::Load);
			writeConstant(currentScope->GetDefinition(label).StackPosition);
		}

		void Generator::StoreVariable(std::string label)
		{
			writeOpcode(Opcode::Store);
			writeConstant(currentScope->GetDefinition(label).StackPosition);
		}

		void Generator::RTConstant(std::string name)
		{
			writeOpcode(Opcode::Constant);
			writeConstantMacroPlaceholder(name);
		}

		void Generator::Constant(std::string constant)
		{
			writeOpcode(Opcode::Constant);
			writeConstant(constant);
		}

		void Generator::Increment(std::string constant)
		{
			writeOpcode(Opcode::Increment);
			writeConstant(constant);
		}

		void Generator::Discard()
		{
			writeOpcode(Opcode::Discard);
		}

		void Generator::Duplicate()
		{
			writeOpcode(Opcode::Duplicate);
		}

		void Generator::Add()
		{
			writeOpcode(Opcode::Add);
		}

		void Generator::Subtract()
		{
			writeOpcode(Opcode::Subtract);
		}

		void Generator::Multiply()
		{
			writeOpcode(Opcode::Multiply);
		}

		void Generator::Divide()
		{
			writeOpcode(Opcode::Divide);
		}

		void Generator::Modulus()
		{
			writeOpcode(Opcode::Modulus);
		}

		void Generator::Negate()
		{
			writeOpcode(Opcode::Negate);
		}

		void Generator::CreateParticle()
		{
			writeOpcode(Opcode::Create);

		}

		void Generator::TransformParticle()
		{
			writeOpcode(Opcode::Transform);
		}

		void Generator::GetParticle()
		{
			writeOpcode(Opcode::Get);
		}

		void Generator::GetPosition()
		{
			writeOpcode(Opcode::Position);
		}

		void Generator::KillParticle()
		{
			writeOpcode(Opcode::Kill);
		}

		void Generator::LoadProperty(std::string property)
		{
			writeOpcode(Opcode::LoadProperty);
			writeConstantPropertyPlaceholder(property);
		}

		void Generator::StoreProperty(std::string property)
		{
			writeOpcode(Opcode::StoreProperty);
			writeConstantPropertyPlaceholder(property);
		}

		void Generator::IntegerToDecimal()
		{

		}

		void Generator::DecimalToInteger()
		{

		}


		void Generator::JumpEqual(std::string label)
		{
			writeOpcode(Opcode::JumpEqual);
			writeConstantPlaceholder(label); 
		}

		void Generator::JumpNotEqual(std::string label)
		{
			writeOpcode(Opcode::JumpNotEqual);
			writeConstantPlaceholder(label); 
		}

		void Generator::JumpGreater(std::string label)
		{
			writeOpcode(Opcode::JumpGreater);
			writeConstantPlaceholder(label); 
		}

		void Generator::JumpGreaterEqual(std::string label)
		{
			writeOpcode(Opcode::JumpGreaterEqual);
			writeConstantPlaceholder(label); 
		}

		void Generator::JumpLess(std::string label)
		{
			writeOpcode(Opcode::JumpLess);
			writeConstantPlaceholder(label); 
		}

		void Generator::JumpLessEqual(std::string label)
		{
			writeOpcode(Opcode::JumpLessEqual);
			writeConstantPlaceholder(label); 
		}

		void Generator::Jump(std::string label)
		{
			writeOpcode(Opcode::Jump);
			writeConstantPlaceholder(label); 
		}


		void Generator::Call(int arguments, std::string label)
		{

		}

		void Generator::Return()
		{

		}

	}
}