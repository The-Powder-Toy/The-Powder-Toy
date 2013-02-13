//Virtual machine

#include <iostream>
#include <fstream>
#include "Machine.h"
#include "Opcodes.h"
#include "Types.h"
#include "simulation/Simulation.h"
#include "X86Native.h"
#include "Exceptions.h"
namespace pim
{
	/*unsigned char * rom;
	int romSize;
	int romMask;

	unsigned char * ram;
	int ramSize;
	int ramMask;

	int programStack;
	int callStack;*/

	VirtualMachine::VirtualMachine(Simulation * simulation) :
		rom(NULL),
		ram(NULL),
		sim(simulation),
		nativeRom(0)
	{

	}

	void VirtualMachine::LoadProgram(std::vector<unsigned char> fileData)
	{
		int lastBit = 0;

		if(!(fileData[0] == 'P' && fileData[1] == 'V' && fileData[2] == 'M' && fileData[3] == '1' && fileData.size() >= 24))
		{
			throw InvalidProgramException();
		}

		int macroSize = 0, propSize = 0, codeSize = 0, heapSize = 0, symtableSize = 0;

		heapSize = fileData[4];
		heapSize |= fileData[5] << 8;
		heapSize |= fileData[6] << 16;
		heapSize |= fileData[7] << 24;

		macroSize = fileData[8];
		macroSize |= fileData[9] << 8;
		macroSize |= fileData[10] << 16;
		macroSize |= fileData[11] << 24;

		propSize = fileData[12];
		propSize |= fileData[13] << 8;
		propSize |= fileData[14] << 16;
		propSize |= fileData[15] << 24;

		symtableSize = fileData[16];
		symtableSize |= fileData[17] << 8;
		symtableSize |= fileData[18] << 16;
		symtableSize |= fileData[19] << 24;

		codeSize = fileData[20];
		codeSize |= fileData[21] << 8;
		codeSize |= fileData[22] << 16;
		codeSize |= fileData[23] << 24;

		if(fileData.size() < 24 + macroSize + propSize + symtableSize + codeSize)
		{
			throw InvalidProgramException();
		}

		//std::vector<std::pair<int, int> > insertions;

		int macroOffset = 24;
		int propOffset = macroOffset+macroSize;
		int symtableOffset = propOffset+propSize;
		int codeOffset = symtableOffset+symtableSize;


		int filePosition = macroOffset;
		while(filePosition + 4 < macroSize + macroOffset)
		{
			std::string macro;
			int macroPosition;
			int macroValue;

			macroPosition = fileData[filePosition++];
			macroPosition |= fileData[filePosition++] << 8;
			macroPosition |= fileData[filePosition++] << 16;
			macroPosition |= fileData[filePosition++] << 24;

			int stringLength = fileData[filePosition++];
			if(filePosition + stringLength > macroSize + macroOffset)
			{
				throw InvalidProgramException();
			}

			macro.insert(macro.begin(), fileData.begin()+filePosition, fileData.begin()+filePosition+stringLength);
			filePosition += stringLength;

			bool resolved = false;
			for(int i = 0; i < PT_NUM; i++)
			{
				if(sim->elements[i].Enabled && sim->elements[i].Identifier == macro)
				{
					macroValue = i;
					resolved = true;
				}
			}
			if(!resolved)
			{
				throw UnresolvedValueException(macro);
			}

			if(macroPosition + 3 >= codeSize)
			{
				throw InvalidProgramException();
			}

			std::cout << "Macro insertion [" << macro << "] at " << macroPosition << " with " << macroValue << std::endl;

			fileData[codeOffset+macroPosition] = macroValue & 0xFF;
			fileData[codeOffset+macroPosition+1] = (macroValue >> 8) & 0xFF;
			fileData[codeOffset+macroPosition+2] = (macroValue >> 16) & 0xFF;
			fileData[codeOffset+macroPosition+3] = (macroValue >> 24) & 0xFF;
			//insertions.push_back(std::pair<int, int>(macroPosition, macroValue));
		}

		filePosition = propOffset;
		while(filePosition + 4 < propSize + propOffset)
		{
			std::string prop;
			int propPosition;
			int propValue;

			propPosition = fileData[filePosition++];
			propPosition |= fileData[filePosition++] << 8;
			propPosition |= fileData[filePosition++] << 16;
			propPosition |= fileData[filePosition++] << 24;

			int stringLength = fileData[filePosition++];
			if(filePosition + stringLength > propSize + propOffset)
			{
				throw InvalidProgramException();
			}

			prop.insert(prop.begin(), fileData.begin()+filePosition, fileData.begin()+filePosition+stringLength);
			filePosition += stringLength;

			bool resolved = false;

			std::vector<StructProperty> properties = Particle::GetProperties();
			for(std::vector<StructProperty>::iterator iter = properties.begin(), end = properties.end(); iter != end; ++iter)
			{
				StructProperty property = *iter;
				std::cout << property.Offset << std::endl;
				if(property.Name == prop &&
					(property.Type == StructProperty::ParticleType ||
						property.Type == StructProperty::Colour ||
						property.Type == StructProperty::Integer ||
						property.Type == StructProperty::UInteger ||
						property.Type == StructProperty::Float)
					)
				{
					propValue = property.Offset;
					resolved = true;
					break;
				}
			}
			if(!resolved)
			{
				throw UnresolvedValueException(prop);
			}

			if(propPosition + 3 >= codeSize)
			{
				throw InvalidProgramException();
			}

			std::cout << "Property insertion [" << prop << "] at " << propPosition << " with " << propValue << std::endl;

			fileData[codeOffset+propPosition] = propValue & 0xFF;
			fileData[codeOffset+propPosition+1] = (propValue >> 8) & 0xFF;
			fileData[codeOffset+propPosition+2] = (propValue >> 16) & 0xFF;
			fileData[codeOffset+propPosition+3] = (propValue >> 24) & 0xFF;
			//insertions.push_back(std::pair<int, int>(macroPosition, macroValue));
		}

		filePosition = symtableOffset;
		while(filePosition + 4 < symtableSize + symtableOffset)
		{
			std::string symbolName;
			int stringLength = 0;
			int symType = 0;
			int position = 0;

			symType = fileData[filePosition++];
			stringLength = fileData[filePosition++];

			if(filePosition + stringLength > symtableSize + symtableOffset)
			{
				printf("%d %d\n", filePosition + stringLength, symtableSize + symtableOffset);
				throw InvalidProgramException();
			}
			symbolName.insert(symbolName.begin(), fileData.begin()+filePosition, fileData.begin()+filePosition+stringLength);
			filePosition += stringLength;

			position = fileData[filePosition++];
			position |= fileData[filePosition++] << 8;
			position |= fileData[filePosition++] << 16;
			position |= fileData[filePosition++] << 24;

			if(symType == 255)
			{
				std::vector<int> types = std::vector<int>();
				int returnType = fileData[filePosition++];
				int typeCount = fileData[filePosition++];
				for(int i = 0; i < typeCount; i++)
				{
					types.push_back(fileData[filePosition++]);
				}

				std::string typeName = "void";
				if(returnType != 255)
					typeName = TypeName(returnType);

				std::cout << "Symbol \"" << symbolName << "\", " << typeName <<  " (" << ArgsName(types) << ") at " << position << std::endl;
				functions.push_back(Function(symbolName, position, returnType != 255, symType, types));
			}
			else
			{
				if(position + 4 > heapSize)
				{
					throw InvalidProgramException();
				}
				std::cout << "Symbol \"" << symbolName << "\", " << TypeName(symType) << " at " << position << std::endl;
				variables.push_back(Variable(symbolName, position, symType));
			}

		}

		std::vector<unsigned char> programData;
		programData.insert(programData.begin(), fileData.begin()+codeOffset, fileData.begin()+codeOffset+codeSize);

		romSize = programData.size();

		for (lastBit = 0; romSize > (1 << lastBit); lastBit++ ) { }
		romSize = 1 << lastBit;
		romMask = romSize - 1;

		rom = new Instruction[romSize];


		int pc = 0;
		int programPosition = 0;

		while(programPosition < programData.size())
		{
			int argSize = 0;
			Instruction instruction;
			instruction.Opcode = programData[programPosition++];
			if(argSize = opcodeArgSize(instruction.Opcode))
			{
				if(argSize == 4 && programPosition+3 < programData.size())
				{
					int tempInt = 0;
					tempInt |= programData[programPosition];
					tempInt |= programData[programPosition+1] << 8;
					tempInt |= programData[programPosition+2] << 16;
					tempInt |= programData[programPosition+3] << 24;


					//std::cout << "Got integer " << tempInt << std::endl;

					if(instruction.Opcode == Opcode::LoadProperty || instruction.Opcode == Opcode::StoreProperty)
					{
						if(tempInt > offsetof(Particle, dcolour))
							throw InvalidProgramException();
					}

					if(instruction.Opcode == Opcode::LoadAdr || instruction.Opcode == Opcode::StoreAdr)
					{
						if(tempInt > heapSize)
							throw InvalidProgramException();
					}

					instruction.Parameter.Integer = tempInt;

					programPosition += 4; 
				}
			}
			else
			{
				instruction.Parameter.Integer = 0;
			}
			rom[pc++] = instruction;
		}
		romSize = pc;

		ramSize = 1024;
		ramMask = ramSize - 1;

		ram = new unsigned char[ramSize];
		programStack = ramSize-1;
		callStack = ramSize-260;

		framePointer = callStack;
		callStack += WORDSIZE;	//Since there's nothing on the stack, it shouldn't point to the item on the bottom

		heap = new unsigned char[heapSize];
		this->heapSize = heapSize;
	}

	int VirtualMachine::opcodeArgSize(int opcode)
	{
		switch(opcode)
		{
		case Opcode::Load:
		case Opcode::LoadAdr:
		case Opcode::Store:
		case Opcode::StoreAdr:
		case Opcode::Constant:
		case Opcode::Increment:
		case Opcode::JumpEqual:
		case Opcode::JumpNotEqual:
		case Opcode::JumpGreater:
		case Opcode::JumpGreaterEqual:
		case Opcode::JumpLess:
		case Opcode::JumpLessEqual:
		case Opcode::Jump:
		case Opcode::Leave:
		case Opcode::LocalEnter:
		case Opcode::LoadProperty:
		case Opcode::StoreProperty:
			return 4;
		case Opcode::Begin:
		case Opcode::Return:
		case Opcode::Discard:
		case Opcode::Duplicate:
		case Opcode::Add:
		case Opcode::Subtract:
		case Opcode::Multiply:
		case Opcode::Divide:
		case Opcode::Modulus:
		case Opcode::Negate:
		case Opcode::Create:
		case Opcode::Transform:
		case Opcode::Get:
		case Opcode::Position:
		case Opcode::Kill:
		case Opcode::ToFloat:
		case Opcode::ToInteger:
			return 0;
		}
	}

	void VirtualMachine::run()
	{
		//std::cout << "CS: " << callStack << " PS: " << programStack << std::endl;
		//std::string names[] = { "Load", "Store", "Constant", "Increment", "Discard", "Duplicate", "Add", "Subtract", "Multiply", "Divide", "Modulus", "Negate", "Create", "Transform", "Get", "Position", "Kill", "JumpEqual", "JumpNotEqual", "JumpGreater", "JumpGreaterEqual", "JumpLess", "JumpLessEqual", "Jump", "Return", "LocalEnter"};

		Word temp1;
		Word temp2;
		Word temp3;
		Word temp4;
		int temp;
		while(programCounter < romSize)
		{
			Word argument = rom[programCounter].Parameter;
			//std::cerr << programCounter << "\t" << names[rom[programCounter].Opcode] << "\t" << argument.Integer << std::endl;//"\t";
			switch(rom[programCounter].Opcode)
			{
			case Opcode::Begin:
				break;
			case Opcode::Load:
				PSPush(CSA(argument.Integer));
				break;
			case Opcode::Store:
				CSA(argument.Integer) = PSPop();
				break;
			case Opcode::Constant:
				PSPush(argument);
				break;
			case Opcode::Increment:
				PS().Integer += argument.Integer;
				break;
			case Opcode::Discard:
				programStack += WORDSIZE;
				break;
			case Opcode::Duplicate:
				PSPush(PS());
				break;
			case Opcode::Add:
				PSPush(PSPop().Integer + PSPop().Integer);
				break;
			case Opcode::Subtract:
				temp1 = PSPop();
				PSPush(PSPop().Integer - temp1.Integer);
				break;
			case Opcode::Multiply:
				PSPush(PSPop().Integer * PSPop().Integer);
				break;
			case Opcode::Divide:
				temp1 = PSPop();
				PSPush(PSPop().Integer / temp1.Integer);
				break;
			case Opcode::Modulus:
				temp1 = PSPop();
				PSPush(PSPop().Integer % temp1.Integer);
				break;
			case Opcode::Negate:
				PS().Integer = -PS().Integer;
				break;
			case Opcode::Create:
				temp1 = PSPop();
				temp2 = PSPop();
				temp3 = PSPop();
				PSPush(sim->create_part(PSPop().Integer, temp3.Integer, temp2.Integer, temp1.Integer));
				break;
			case Opcode::Transform:
				PSPop();
				PSPop();
				PSPush((Word)-1);
				break;
			case Opcode::Get:
				temp1 = PSPop();
				temp2 = PSPop();
				if(temp1.Integer < 0 || temp1.Integer >= YRES || temp2.Integer < 0 || temp2.Integer >= XRES || !(temp = sim->pmap[temp1.Integer][temp2.Integer]))
				{
					PSPush(-1);
					break;
				}
				PSPush(temp>>8);
				break;
			case Opcode::Position:
				temp1 = PSPop();
				if(temp1.Integer < 0 || temp1.Integer >= NPART || !sim->parts[temp1.Integer].type)
				{
					PSPush(-1);
					PSPush(-1);
					break;
				}
				PSPush((int)sim->parts[temp1.Integer].x);
				PSPush((int)sim->parts[temp1.Integer].y);
				break;
			case Opcode::Kill:
				sim->kill_part(PSPop().Integer);
				PSPush((Word)0);
				break;
			case Opcode::LoadProperty:
				PSPush(PPROP(PSPop().Integer, argument.Integer));
				break;
			case Opcode::StoreProperty:
				temp1 = PSPop();
				PPROP(temp1.Integer, argument.Integer) = PSPop();
				break;
			case Opcode::JumpEqual:
				if(PSPop().Integer == PSPop().Integer)
					programCounter = argument.Integer-1;
				break;
			case Opcode::JumpNotEqual:
				if(PSPop().Integer != PSPop().Integer)
					programCounter = argument.Integer-1;
				break;
			case Opcode::JumpGreater:
				temp1 = PSPop();
				if(PSPop().Integer > temp1.Integer)
					programCounter = argument.Integer-1;
				break;
			case Opcode::JumpGreaterEqual:
				temp1 = PSPop();
				if(PSPop().Integer >= temp1.Integer)
					programCounter = argument.Integer-1;
				break;
			case Opcode::JumpLess:
				temp1 = PSPop();
				if(PSPop().Integer < temp1.Integer)
					programCounter = argument.Integer-1;
				break;
			case Opcode::JumpLessEqual:
				temp1 = PSPop();
				if(PSPop().Integer <= temp1.Integer)
					programCounter = argument.Integer-1;
				break;
			case Opcode::Jump:
				programCounter = argument.Integer-1;
				break;
			case Opcode::Leave:
				callStack += argument.Integer;
				break;
			case Opcode::Return:
				break;
			case Opcode::LocalEnter:
				callStack -= argument.Integer;
				break;
			}
			//std::cout << programStack << std::endl;
			programCounter++;
		}
		//std::cout << "CS: " << callStack << " PS: " << programStack << std::endl;
	}

	void VirtualMachine::Compile()
	{
		Native * native = new X86Native();
		if(nativeRom)
			delete[] nativeRom;
		if(nativeStack)
			delete[] nativeStack;

		nativeStack = new unsigned char [1024*1024];

		std::vector<unsigned char> executableCode = native->Compile(sim, nativeStack + (1024*512), heap, rom, romSize);
		virtualXToNative = native->GetAddressMap();
		nativeRom = new unsigned char[executableCode.size()];

		std::copy(executableCode.begin(), executableCode.end(), nativeRom);

		delete native;
	}

	void * VirtualMachine::GetNativeEntryPoint(std::string entryPoint, std::string returnType, std::string argTypes)
	{
		for(std::vector<Function>::iterator iter = functions.begin(), end = functions.end(); iter != end; ++iter)
		{
			Function f = *iter;
			if(f.Name == entryPoint)
			{
				printf("Got native entry point for %s at %p\n", entryPoint.c_str(), nativeRom+virtualXToNative[f.Position]);
				fflush(stdout);
				return nativeRom+virtualXToNative[f.Position];
			}
		}
		throw FunctionNotFoundException(entryPoint, returnType, argTypes);
	}

	/*void * VirtualMachine::GetNativeEntryPoint(intptr_t entryPoint)
	{
		return nativeRom+entryPoint;
	}*/

	int VirtualMachine::GetEntryPoint(std::string entryPoint, std::string returnType, std::string argTypes)
	{
		for(std::vector<Function>::iterator iter = functions.begin(), end = functions.end(); iter != end; ++iter)
		{
			Function f = *iter;
			if(f.Name == entryPoint)
			{
				return f.Position;
				//if()
			}
		}
		throw FunctionNotFoundException(entryPoint, returnType, argTypes);
	}

	/*void VirtualMachine::Run(std::string entryPoint)
	{
	}*/

	void VirtualMachine::Run(int entryPoint)
	{
		programCounter = entryPoint;
		run();
	}
}
