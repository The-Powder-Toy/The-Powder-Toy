#include "Machine.h"
#include <map>

namespace pim
{
	class Native
	{
	protected:
		std::vector<unsigned char> nativeRom;
		void emit(std::string opcode)
		{
			unsigned char c1, c2;
			unsigned char v;
			const char * string = opcode.c_str();

			while (true)
			{
				c1 = string[0];
				c2 = string[1];

				v = (hex( c1 ) << 4) | hex(c2);
				nativeRom.push_back(v);

				if (!string[2])
				{
					break;
				}
				string += 3;
			}
		}

		unsigned char hex(char c)
		{
			if (c >= 'a' && c <= 'f')
				return 10 + c - 'a';
			if (c >= 'A' && c <= 'F')
				return 10 + c - 'A';
			if (c >= '0' && c <= '9')
				return c - '0';
			return 0;
		}
		virtual void emitConstantS(short int constant) {}		//Single
		virtual void emitConstantD(long int constant) {}		//Double
		virtual void emitConstantP(intptr_t constant) {}		//Pointer
		virtual void emitConstantQ(long long int constant) {}	//Quad
		virtual void emitCall(intptr_t objectPtr, intptr_t functionAddress, int stackSize) {}
		virtual void emitPlaceholder(int virtualAddress) {}
	public:
		Native() : nativeRom()  {}
		virtual std::map<int, int> GetAddressMap() {}
		virtual std::vector<unsigned char> Compile(Simulation * sim, unsigned char * machineStack, unsigned char * heap, Instruction * rom, int romSize) {}

	};
}