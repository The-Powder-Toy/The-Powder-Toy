#pragma once

#include <vector>
#include <string>
#include <cstring>

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
	class InvalidProgramException: public std::exception
	{
	public:
		InvalidProgramException() { } 
		const char * what() const throw()
		{
			return "Invalid program";
		}
		~InvalidProgramException() throw() {};
	};
	class UnresolvedValueException: public std::exception
	{
		char * error;
	public:
		UnresolvedValueException(std::string value) {
			error = strdup(std::string("Unresolved value: " + value).c_str());
		} 
		const char * what() const throw()
		{
			return error;
		}
		~UnresolvedValueException() throw() {};
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

		unsigned char * compiledRom;
		int compiledRomSize;

		unsigned char * ram;
		int ramSize;
		int ramMask;

		#define CSA(argument) (*((Word*)&ram[framePointer-argument]))
		#define CS() (*((Word*)&ram[callStack]))
		#define PS() (*((Word*)&ram[programStack]))
		#define PPROP(index, property) (*((Word*)(((char*)&sim->parts[(index)])+property)))

		int programStack;	//Points to the item on top of the Program Stack
		int callStack;		//Points to the item on top of the call stack
		int framePointer;	//Points to the bottom (first item) on the current frame of the call stack

		//Instruction * instructions;

		int programCounter;

		void emit(std::string opcode);
		void emit(int constant);
	public:
		VirtualMachine(Simulation * sim);
		int OpcodeArgSize(int opcode);
		void LoadProgram(std::vector<unsigned char> programData);
		void Run();
		void Compile();
		void CallCompiled(std::string entryPoint);
		void CallCompiled(int entryPoint);
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
