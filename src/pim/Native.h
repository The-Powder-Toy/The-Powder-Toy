#include "Machine.h"

namespace pim
{
	class Native
	{
	public:
		Native()  {}
		virtual std::vector<unsigned char> Compile(Simulation * sim, Instruction * rom, int romSize) {}
	};
}