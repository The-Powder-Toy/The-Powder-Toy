namespace pim
{
	struct Opcode
	{
		enum
		{
			#define OPDEF(name) name, 
			#include "Opcodes.inl"
			#undef OPDEF
		};
	};
}
