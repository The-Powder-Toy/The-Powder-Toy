#include <vector>
#include <map>
#include "Native.h"

namespace pim
{
	class X86Native : public Native
	{
	public:
		X86Native() : Native() {}
		virtual std::vector<unsigned char> Compile(Simulation * sim, unsigned char * machineStack, unsigned char * heap, Instruction * rom, int romSize);
		virtual std::map<int, int> GetAddressMap() { return virtualToNative; }
	protected:
		virtual void emitConstantD(long int constant);
		virtual void emitConstantP(intptr_t constant);
		virtual void emitCall(intptr_t objectPtr, intptr_t functionAddress, int stackSize);
		virtual void emitPlaceholder(int virtualAddress);
		std::map<int, int> virtualToNative;
		std::map<int, int> placeholders;
	};
}