#include "Opcodes.h"
#include "X86Native.h"

namespace pim 
{
	std::vector<unsigned char> X86Native::Compile(Instruction * rom, int romSize)
	{
	#if defined(X86) && !defined(_64BIT)
		int programCounter = 0;
		nativeRom.clear();
		while(programCounter < romSize)
		{
			Word argument = rom[programCounter].Parameter;
			switch(rom[programCounter].Opcode)
			{
			case Opcode::Load:
				emit("83 EE 04"); 								//sub esi, 4
				//Load value at base stack + offset into eax
				emit("8B 85");									//mov eax, [ebp+offset]
				emit((int) (argument.Integer));
				//Store value in eax onto top of program stack
				emit("89 06");									//mov [esi], eax
				break;
			case Opcode::Store:
				//Load value on top of the program stack into eax
				emit("8B 06");									//mov eax, [esi]
				//Load value in eax into memory
				emit("89 85");									//mov [ebp+offset], eax
				emit((int) (argument.Integer));
				emit("83 C6 04");								//add esi, 4
				break;
			case Opcode::Constant:
				emit("83 EE 04");								//sub esi, 4
				emit("C7 06");									//mov [esi], dword ptr constant
				emit((int) (argument.Integer));
				break;
			case Opcode::Increment:
				emit("81 06");									//add [esi], dword ptr constant
				emit((int) (argument.Integer));
				break;
			case Opcode::Discard:
				emit("83 C6 04");								//add esi, 4
				break;
			case Opcode::Duplicate:
				//Copy value on stack into register
				emit("8B 06"); 									//mov eax, [esi]
				//Adjust program stack pointer
				emit("83 EE 04"); 								//sub esi, 4
				//Move value in eax into program stack
				emit("89 06");									//mov [esi], eax
				break;
			case Opcode::Add:
				emit("8B 06"); 									//mov eax, [esi]
				emit("83 C6 04");								//add esi, 4
				emit("01 06"); 									//add [esi], eax
				break;
			case Opcode::Subtract:
				emit("8B 06"); 									//mov eax, [esi]
				emit("83 C6 04");								//add esi, 4
				emit("29 06");	 								//sub [esi], eax
				break;
			case Opcode::Multiply:
				emit("8B 46 04"); 								//mov eax, [esi+4]
				emit("F7 2E"); 									//imul [esi]
				emit("83 C6 04");								//add esi, 4
				emit("89 06");									//mov [esi], eax
				break;
			case Opcode::Divide:
				emit("8B 46 04");								//mov eax [esi+4]
				emit("99");										//cdq
				emit("F7 3E"); 									//idiv [esi]
				emit("83 C6 04");								//add esi, 4
				emit("89 06");									//mov [esi], eax
				break;
			case Opcode::Modulus:
				emit("8B 46 04");								//mov eax [esi+4]
				emit("99");										//cdq
				emit("F7 3E"); 									//idiv [esi]
				emit("83 C6 04");								//add esi, 4
				emit("89 16");									//mov [esi], edx
				break;
			case Opcode::Negate:
				emit("F7 1E");									//neg [esi]
				break;
			case Opcode::Create:
				//temp1 = PSPop();
				//temp2 = PSPop();
				//temp3 = PSPop();
				//PSPush(sim->create_part(PSPop().Integer, temp3.Integer, temp2.Integer, temp1.Integer));
				break;
			case Opcode::Transform:
				//PSPop();
				//PSPop();
				//PSPush((Word)-1);
				break;
			case Opcode::Get:
				//temp1 = PSPop();
				//temp2 = PSPop();
				//if(temp1.Integer < 0 || temp1.Integer >= YRES || temp2.Integer < 0 || temp2.Integer >= XRES || !(temp = sim->pmap[temp1.Integer][temp2.Integer]))
				//{
				//	PSPush(-1);
				//	break;
				//}
				//PSPush(temp>>8);
				{
					int propertyOffset = argument.Integer;
					int partsArray = 0;
					emit("8B 06");								//mov eax, [esi]
																//mov ecx, [esi+4]

																//and eax, 348
																//and ecx, 612

																//imul eax, 612, eax
					emit("8B 46 04");							//add eax, ecx
					emit("8B 81");								//mov eax, [eax*4+pmap]
																//sar eax, 8
					emit("89 06");								//mov [esi+4], eax	#Copy eax onto stack
					emit("83 C6 08"); 							//add esi, 4
				}
				break;
			case Opcode::Position:
				//temp1 = PSPop();
				//if(temp1.Integer < 0 || temp1.Integer >= NPART || !sim->parts[temp1.Integer].type)
				//{
				//	PSPush(-1);
				//	PSPush(-1);
				//	break;
				//}
				//PSPush((int)sim->parts[temp1.Integer].x);
				//PSPush((int)sim->parts[temp1.Integer].y);
				break;
			case Opcode::Kill:
				//sim->kill_part(PSPop().Integer);
				//PSPush((Word)0);
				break;
			case Opcode::LoadProperty:
				{
					int propertyOffset = argument.Integer;
					int partsArray = 0;
					emit("8B 06");								//mov eax, [esi]	#Load index from stack
					emit("C1 E0 03");							//sal eax, 3		#Shift index left (multiply by 8)	
					emit("8D 14 C5");							//lea edx, [eax*8]	#Mutiply by 8 again and copy into edx		//Size of 56 is represented by (1*(8^2))-(1*8)
					emit("89 D1");								//mov ecx, edx		
					emit("29 C1");								//sub ecx, eax		#Subtract index*8^2 by index*8 to get the index*56
					emit("8B 81");								//mov eax, [ecx+propertyOffset+partsArray]	#Copy value at index+baseAddress+propertyOffset into eax
					emit(partsArray+propertyOffset);
					emit("89 06");								//mov [esi], eax	#Copy eax onto stack
				}
				break;
			case Opcode::StoreProperty:
				{
					int propertyOffset = argument.Integer;
					int partsArray = 0;
					emit("8B 06");								//mov eax, [esi]
					emit("C1 E0 03");							//sal eax, 3
					emit("8D 14 C5");							//lea edx, [eax*8]
					emit("89 D1");								//mov ecx, edx
					emit("29 C1");								//sub ecx, eax
					emit("8B 46 04");							//mov eax, [esi+4]
					emit("89 81");								//mov [ecx+propertyOffset+partsArray], eax
					emit(partsArray+propertyOffset);
					emit("83 C6 08"); 							//add esi, 8
				}
				break;
			case Opcode::JumpEqual:
				emit("83 C6 08"); 								//add esi, 8
				emit("8B 46 FC"); 								//mov eax, [esi-4]
				emit("3B 46 F8");								//cmp eax, [esi-8]
				emit("75 08");									//jne 8
				emit("E9 01"); 									//jmp [0x12345678]
				emit((int)0);
				break;
			case Opcode::JumpNotEqual:
				emit("83 C6 04"); 								//add esi, 8
				emit("8B 46 FC");								//mov eax, [esi-4]
				emit("3B 46 F8"); 								//cmp eax, [esi-8]
				emit("74 08");									//je 8
				emit("E9 01"); 									//jmp [0x12345678]
				emit((int)0);
				break;
			case Opcode::JumpGreater:
				emit("83 C6 08");								//add esi 8
				emit("8B 46 FC"); 								//mov eax, [esi-4]
				emit("3B 46 F8"); 								//cmp eax, [esi-8]
				emit("7E 08");									//jng 8
				emit("E9 01");									//jmp [0x12345678]
				emit((int)0);
				break;
			case Opcode::JumpGreaterEqual:
				emit("83 C6 08");								//add esi 8
				emit("8B 46 FC");								//mov eax, [esi-4]
				emit("3B 46 F8");								//cmp eax, [esi-8]
				emit("7C 08");									//jnge +6
				emit("E9 01");									//jmp [0x12345678]
				emit((int)0);
				break;
			case Opcode::JumpLess:
				emit("83 C6 08");								//add esi 8
				emit("8B 46 FC");								//mov eax, [esi-4]
				emit("3B 46 F8");								//cmp eax, [esi-8]
				emit("7D 08");									//jnl +6
				emit("E9 01"); 									//jmp [0x12345678]
				emit((int)0);
				break;
			case Opcode::JumpLessEqual:
				emit("83 C6 08"); 								//add esi 8
				emit("8B 46 FC"); 								//mov eax, [esi-4]
				emit("3B 46 F8"); 								//cmp eax, [esi-8]
				emit("7F 08"); 									//jnle +6
				emit("E9 01");									//jmp [0x12345678]
				emit((int)0);
				break;
			case Opcode::Jump:
				//programCounter = argument.Integer-1;
				break;
			case Opcode::Return:
				emit("81 C7");									//add edi, constant
				emit(argument.Integer);
				break;
			case Opcode::LocalEnter:
				emit("81 EF");									//sub edi constant
				emit(argument.Integer);
				break;
			}
			//std::cout << programStack << std::endl;
			programCounter++;
		}
		//std::cout << "CS: " << callStack << " PS: " << programStack << std::endl;
		return nativeRom;
	#endif
	}

	void X86Native::emit(std::string opcode)
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

	void X86Native::emit(int constant)
	{
		nativeRom.push_back(constant & 0xFF);
		nativeRom.push_back((constant >> 8) & 0xFF);
		nativeRom.push_back((constant >> 16) & 0xFF);
		nativeRom.push_back((constant >> 24) & 0xFF);
	}

	unsigned char X86Native::hex(char c)
	{
		if (c >= 'a' && c <= 'f')
			return 10 + c - 'a';
		if (c >= 'A' && c <= 'F')
			return 10 + c - 'A';
		if (c >= '0' && c <= '9')
			return c - '0';
		return 0;
	}

}