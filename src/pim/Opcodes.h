namespace pim
{
	struct Opcode
	{
		public:
		enum
		{
			#define OPDEF(name) name, 
			#include "Opcodes.inl"
			#undef OPDEF
		};
	};
}