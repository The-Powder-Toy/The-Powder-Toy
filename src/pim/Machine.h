#pragma once

#include <vector>
#include <string>

class Simulation;
namespace pim
{
	union Word
	{
		int Integer;
		float Decimal;

		Word(int integer) : Integer(integer) {}
		Word(float decimal) : Decimal(decimal) {}
		Word() {}
	};
	struct Instruction
	{
		int Opcode;
		Word Parameter;
	};
	class VirtualMachine
	{

		#define WORDSIZE 4

		//#define OPDEF(name) void op##name(int parameter);
		//#include "Opcodes.inl"
		//#undef OPDEF

		Simulation * sim;

		Instruction * rom;
		int romSize;
		int romMask;

		unsigned char * ram;
		int ramSize;
		int ramMask;

		#define CSA(argument) (*((Word*)&ram[framePointer-argument]))
		#define CS() (*((Word*)&ram[callStack]))
		#define PS() (*((Word*)&ram[programStack]))
		#define PPROP(index, property) (*((Word*)(&sim->parts[(index)]+property)))

		int programStack;	//Points to the item on top of the Program Stack
		int callStack;		//Points to the item on top of the call stack
		int framePointer;	//Points to the bottom (first item) on the current frame of the call stack

		//Instruction * instructions;

		int programCounter;


	public:
		VirtualMachine(Simulation * sim);
		int OpcodeArgSize(int opcode);
		void LoadProgram(std::vector<unsigned char> programData);
		void Run();
		void Call(std::string entryPoint);
		void Call(int entryPoint);
		inline void PSPush(Word word)
		{
			programStack -= WORDSIZE;
			PS() = word;
		}

		inline Word PSPop()
		{
			Word word = PS();
			programStack += WORDSIZE;
			return word;
		} 

		inline void CSPush(Word word)
		{
			callStack -= WORDSIZE;
			CS() = word;
		}

		inline Word CSPop()
		{
			Word word = CS();
			callStack += WORDSIZE;
			return word;
		} 
	};
}