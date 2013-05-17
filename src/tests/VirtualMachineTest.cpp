#ifdef TEST

#include <iostream>
#include "virtualmachine/VirtualMachine.h"

int main(int argc, char * argv[])
{
	vm::VirtualMachine * vm = new vm::VirtualMachine(2);
	vm->LoadProgram("test.qvm");
	while(true)
	{
		vm->Call(0/*, 0, 88, 12*/);
		std::cout << "Return value: " << vm->Pop<vm::uint4_t>() << std::endl;
	}
}
#endif
