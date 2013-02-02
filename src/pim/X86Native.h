#include <vector>
#include <map>
#include "Native.h"

namespace pim
{
	class X86Native : public Native
	{
	public:
		X86Native() : Native(), nativeRom() {}
		virtual std::vector<unsigned char> Compile(Simulation * sim, Instruction * rom, int romSize);
	private: 
		void emit(std::string opcode);
		void emit(int constant);
		void emitCall(intptr_t objectPtr, intptr_t functionAddress, int stackSize);
		void emitPlaceholder(int virtualAddress);
		unsigned char hex(char c);
		std::vector<unsigned char> nativeRom;
		std::map<int, int> virtualToNative;
		std::map<int, int> placeholders;
	};
}