#pragma once

#include <vector>
#include <string>
#include <cstring>
#include <map>

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

		class Variable
		{
		public:
			std::string Name;
			int Type;
			int Position;
			Variable(std::string name, int position, int type)
			{
				Name = name;
				Position = position;
				Type = type;
			}
		};
		class Function
		{
		public:
			std::string Name;
			bool HasReturn;
			int ReturnType;
			std::vector<int> ArgTypes;
			int Position;
			Function(std::string name, int position, bool hasReturn, int returnType, std::vector<int> argTypes)
			{
				Name = name;
				Position = position;
				HasReturn = hasReturn;
				ReturnType = returnType;
				ArgTypes = argTypes;
			}
		};
		#define WORDSIZE 4

		//#define OPDEF(name) void op##name(int parameter);
		//#include "Opcodes.inl"
		//#undef OPDEF

		Simulation * sim;

		Instruction * rom;
		int romSize;
		int romMask;

		unsigned char * nativeRom;
		unsigned char * nativeStack; 

		unsigned char * compiledRom;
		int compiledRomSize;

		unsigned char * ram;
		int ramSize;
		int ramMask;
		unsigned char * heap;
		int heapSize;

		#define CSA(argument) (*((Word*)&ram[framePointer-argument]))
		#define CS() (*((Word*)&ram[callStack]))
		#define PS() (*((Word*)&ram[programStack]))
		#define PPROP(index, property) (*((Word*)(((char*)&sim->parts[(index)])+property)))

		int programStack;	//Points to the item on top of the Program Stack
		int callStack;		//Points to the item on top of the call stack
		int framePointer;	//Points to the bottom (first item) on the current frame of the call stack

		//Instruction * instructions;

		std::vector<Function> functions;
		std::vector<Variable> variables;
		std::map<int, int> virtualXToNative;

		int programCounter;
		void run();
		int opcodeArgSize(int opcode);
	public:
		VirtualMachine(Simulation * sim);
		void LoadProgram(std::vector<unsigned char> programData);
		void Compile();
		void * GetNativeEntryPoint(std::string entryPoint, std::string returnType, std::string argTypes);
		int GetEntryPoint(std::string entryPoint, std::string returnType, std::string argTypes);
		void Run(int entryPoint);
		inline bool IsCompiled()
		{
			return nativeRom != 0;
		}
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