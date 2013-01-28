#include <vector>
#include "Native.h"

namespace pim
{
	class X86Native : public Native
	{
	public:
		X86Native() : Native(), nativeRom() {}
		virtual std::vector<unsigned char> Compile(Instruction * rom, int romSize);
	private: 
		void emit(std::string opcode);
		void emit(int constant);
		unsigned char hex(char c);
		std::vector<unsigned char> nativeRom;
	};
}