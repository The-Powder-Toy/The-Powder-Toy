//Virtual machine

#include <iostream>
#include "Machine.h"
#include "Opcodes.h"
#include "simulation/Simulation.h"
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
		sim(simulation)
	{

	}

	void VirtualMachine::LoadProgram(std::vector<unsigned char> fileData)
	{
		int lastBit = 0;

		if(!(fileData[0] == 'P' && fileData[1] == 'V' && fileData[2] == 'M' && fileData[3] == '1' && fileData.size() >= 16))
		{
			throw InvalidProgramException();
		}

		int macroSize = 0, propSize = 0, codeSize = 0;
		macroSize = fileData[4];
		macroSize |= fileData[5] << 8;
		macroSize |= fileData[6] << 16;
		macroSize |= fileData[7] << 24;

		propSize = fileData[8];
		propSize |= fileData[9] << 8;
		propSize |= fileData[10] << 16;
		propSize |= fileData[11] << 24;

		codeSize = fileData[12];
		codeSize |= fileData[13] << 8;
		codeSize |= fileData[14] << 16;
		codeSize |= fileData[15] << 24;

		if(fileData.size() < 16 + macroSize + propSize + codeSize)
		{
			throw InvalidProgramException();
		}

		//std::vector<std::pair<int, int> > insertions;

		int macroOffset = 16;
		int propOffset = macroOffset+macroSize;
		int codeOffset = propOffset+propSize;


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
			if(argSize = OpcodeArgSize(instruction.Opcode))
			{
				if(argSize == 4 && programPosition+3 < programData.size())
				{
					int tempInt = 0;
					tempInt |= programData[programPosition];
					tempInt |= programData[programPosition+1] << 8;
					tempInt |= programData[programPosition+2] << 16;
					tempInt |= programData[programPosition+3] << 24;


					std::cout << "Got integer " << tempInt << std::endl;

					if(instruction.Opcode == Opcode::LoadProperty || instruction.Opcode == Opcode::StoreProperty)
					{
						if(tempInt > offsetof(Particle, dcolour))
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
	}

	int VirtualMachine::OpcodeArgSize(int opcode)
	{
		switch(opcode)
		{
		case Opcode::Load:
		case Opcode::Store:
		case Opcode::Constant:
		case Opcode::Increment:
		case Opcode::JumpEqual:
		case Opcode::JumpNotEqual:
		case Opcode::JumpGreater:
		case Opcode::JumpGreaterEqual:
		case Opcode::JumpLess:
		case Opcode::JumpLessEqual:
		case Opcode::Jump:
		case Opcode::Return:
		case Opcode::LocalEnter:
		case Opcode::LoadProperty:
		case Opcode::StoreProperty:
			return 4;
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
			return 0;
		}
	}

	void VirtualMachine::Run()
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
			case Opcode::Return:
				callStack += argument.Integer;
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
		while(programCounter < romSize)
		{
			Word argument = rom[programCounter].Parameter;
			switch(rom[programCounter].Opcode)
			{
			case Opcode::Load:
				emit("83 EF 04"); //sub edi 4

				//Load value at base stack + offset into eax
				emit("8B 85");	//mov eax [ebp+ram+offset]
				emit((intptr_t) (ram - argument.Integer));
				
				//Store value in eax onto top of program stack
				emit("89 07");	//mov [edi], eax
				emit((intptr_t) (ram));
				break;
			case Opcode::Store:
				//Load value on top of the program stack into eax
				emit("8B 07");	//mov eax [edi]
				emit((intptr_t) (ram));

				//Load value in eax onto top of program stack
				emit("89 85");	//mov [ebp+ram+offset], eax
				emit((intptr_t) (ram - argument.Integer));

				emit("83 C7 04"); //add edi 4
				break;
			case Opcode::Constant:
				emit("83 EF 04"); //sub edi 4
				
				emit("C7 07"); //mov [edi] constant
				emit((int) (argument.Integer));
				break;
			case Opcode::Increment:
				emit("81 07");	//add [edi] constant
				emit((int) (argument.Integer));
				break;
			case Opcode::Discard:
				emit("83 C7 04"); //add edi 4
				break;
			case Opcode::Duplicate:
				//Copy value on stack into register
				emit("8B 07"); //mov eax [edi]
				//Adjust program stack pointer
				emit("83 EF 04"); //sub edi 4
				//Move value in eax into program stack
				emit("89 07"); //mov [edi], eax
				break;
			case Opcode::Add:
				emit("8B 07"); //mov eax [edi]
				emit("01 47 04"); //add [edi+4] eax
				emit("83 C7 04"); //add edi 4
				break;
			case Opcode::Subtract:
				emit("8B 07"); //mov eax [edi]
				emit("29 47 04"); //sub [edi+4] eax
				emit("83 C7 04"); //add edi 4
				break;
			case Opcode::Multiply:
				emit("8B 47 04"); //mov eax [edi+4]
				emit("F7 2F"); //imul [edi]
				emit("89 47 04"); //mov [edi+4] eax
				emit("83 C7 04"); //add edi 4
				break;
			case Opcode::Divide:
				emit("8B 47 04");//mov eax [edi+4]
				emit("99"); //cdq
				emit("F7 3F"); //idiv [edi]
				emit("89 47 04"); //mov [edi+4] eax
				emit("83 C7 04"); //add edi 4
				break;
			case Opcode::Modulus:
				emit("8B 47 04"); // mov eax [edi+4]
				emit("99"); // cdq
				emit("F7 3F"); // idiv [edi]
				emit("89 57 04"); // mov [edi+4] edx
				emit("83 C7 04"); //add edi 4
				break;
			case Opcode::Negate:
				emit("F7 1F"); //neg [edi]
				break;
			case Opcode::Create:
				//temp1 = PSPop();
				//temp2 = PSPop();
				//temp3 = PSPop();
				//PSPush(sim->create_part(PSPop().Integer, temp3.Integer, temp2.Integer, temp1.Integer));
				break;
			case Opcode::Transform:
				//PSPop();
				//PSPop();
				//PSPush((Word)-1);
				break;
			case Opcode::Get:
				//temp1 = PSPop();
				//temp2 = PSPop();
				//if(temp1.Integer < 0 || temp1.Integer >= YRES || temp2.Integer < 0 || temp2.Integer >= XRES || !(temp = sim->pmap[temp1.Integer][temp2.Integer]))
				//{
				//	PSPush(-1);
				//	break;
				//}
				//PSPush(temp>>8);
				break;
			case Opcode::Position:
				//temp1 = PSPop();
				//if(temp1.Integer < 0 || temp1.Integer >= NPART || !sim->parts[temp1.Integer].type)
				//{
				//	PSPush(-1);
				//	PSPush(-1);
				//	break;
				//}
				//PSPush((int)sim->parts[temp1.Integer].x);
				//PSPush((int)sim->parts[temp1.Integer].y);
				break;
			case Opcode::Kill:
				//sim->kill_part(PSPop().Integer);
				//PSPush((Word)0);
				break;
			case Opcode::LoadProperty:
				//PSPush(PPROP(PSPop().Integer, argument.Integer));
				break;
			case Opcode::StoreProperty:
				//temp1 = PSPop();
				//PPROP(temp1.Integer, argument.Integer) = PSPop();
				break;
			case Opcode::JumpEqual:
				emit("83 C7 04"); //add edi 8
				emit("8B 47 FC"); //mov eax, dword ptr [edi-4]
				emit("3B 47 F7"); //cmp eax, dword ptr [edi-8]
				emit("75 06"); //jne +6
				emit("FF 25"); //jmp [0x12345678]
				emit(0);
				break;
			case Opcode::JumpNotEqual:
				emit("83 C7 04"); //add edi 8
				emit("8B 47 FC"); //mov eax, dword ptr [edi-4]
				emit("3B 47 F7"); //cmp eax, dword ptr [edi-8]
				emit("74 06"); //je +6
				emit("FF 25"); //jmp [0x12345678]
				emit(0);
				break;
			case Opcode::JumpGreater:
				emit("83 C7 04"); //add edi 8
				emit("8B 47 FC"); //mov eax, dword ptr [edi-4]
				emit("3B 47 F7"); //cmp eax, dword ptr [edi-8]
				emit("7E 06"); //jng +6
				emit("FF 25"); //jmp [0x12345678]
				emit(0);
				break;
			case Opcode::JumpGreaterEqual:
				emit("83 C7 04"); //add edi 8
				emit("8B 47 FC"); //mov eax, dword ptr [edi-4]
				emit("3B 47 F7"); //cmp eax, dword ptr [edi-8]
				emit("7C 06"); //jnge +6
				emit("FF 25"); //jmp [0x12345678]
				emit(0);
				break;
			case Opcode::JumpLess:
				emit("83 C7 04"); //add edi 8
				emit("8B 47 FC"); //mov eax, dword ptr [edi-4]
				emit("3B 47 F7"); //cmp eax, dword ptr [edi-8]
				emit("7D 06"); //jnl +6
				emit("FF 25"); //jmp [0x12345678]
				emit(0);
				break;
			case Opcode::JumpLessEqual:
				emit("83 C7 04"); //add edi 8
				emit("8B 47 FC"); //mov eax, dword ptr [edi-4]
				emit("3B 47 F7"); //cmp eax, dword ptr [edi-8]
				emit("7F 06"); //jnle +6
				emit("FF 25"); //jmp [0x12345678]
				emit(0);
				break;
			case Opcode::Jump:
				//programCounter = argument.Integer-1;
				break;
			case Opcode::Return:
				emit("81 C6"); //add esi constant
				emit(argument.Integer);
				break;
			case Opcode::LocalEnter:
				emit("81 EE"); //sub esi constant
				emit(argument.Integer);
				break;
			}
			//std::cout << programStack << std::endl;
			programCounter++;
		}
		//std::cout << "CS: " << callStack << " PS: " << programStack << std::endl;
	}

	void VirtualMachine::emit(std::string opcode)
	{

	}

	void VirtualMachine::emit(int constant)
	{

	}

	void VirtualMachine::CallCompiled(std::string entryPoint)
	{

	}

	void VirtualMachine::CallCompiled(int entryPoint)
	{

	}

	void VirtualMachine::Call(std::string entryPoint)
	{

	}

	void VirtualMachine::Call(int entryPoint)
	{
		programCounter = entryPoint;
		Run();
	}
}
