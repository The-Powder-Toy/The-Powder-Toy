#include "Machine.h"

namespace pim
{
	class Native
	{
	public:
		Native()  {}
		virtual std::vector<unsigned char> Compile(Instruction * rom, int romSize) {}
	};
}