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

	void VirtualMachine::LoadProgram(std::vector<unsigned char> programData)
	{
		int lastBit = 0;

		romSize = programData.size();

		for (lastBit = 0; romSize > (1 << lastBit); lastBit++ ) { }
		romSize = 1 << lastBit;
		romMask = romSize - 1;

		rom = new Instruction[romSize];

		int programPosition = 0;
		int pc = 0;
		while(programPosition < programData.size())
		{
			int argSize = 0;
			Instruction instruction;
			instruction.Opcode = programData[programPosition++];
			if(argSize = OpcodeArgSize(instruction.Opcode))
			{
				if(argSize == 4)
				{
					int tempInt = 0;
					tempInt |= programData[programPosition];
					tempInt |= programData[programPosition+1] << 8;
					tempInt |= programData[programPosition+2] << 16;
					tempInt |= programData[programPosition+3] << 24;


					std::cout << "Got integer " << tempInt << std::endl;
					//std::cout << "Got byte " << (int)(programData[programPosition]) << std::endl;
					//std::cout << "Got byte " << (int)(programData[programPosition+1]) << std::endl;
					//std::cout << "Got byte " << (int)(programData[programPosition+2]) << std::endl;
					//std::cout << "Got byte " << (int)(programData[programPosition+3]) << std::endl;

					//*(int*)&rom[programPosition] = tempInt;
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
		//std::copy(programData.begin(), programData.end(), rom);


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

	void VirtualMachine::Call(std::string entryPoint)
	{

	}

	void VirtualMachine::Call(int entryPoint)
	{
		programCounter = entryPoint;
		Run();
	}
}