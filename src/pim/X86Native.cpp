#include "Opcodes.h"
#include "X86Native.h"
#include "simulation/Simulation.h"

namespace pim 
{
	std::vector<unsigned char> X86Native::Compile(Simulation * sim, Instruction * rom, int romSize)
	{
	#if defined(X86) && !defined(_64BIT)
		int programCounter = 0;
		nativeRom.clear();
		unsigned char * esi = new unsigned char[1024*1024];//malloc(1024*1024);
		esi += 512;
		//int * esi = malloc(1024*1024);
		emit("BE");					//mov esi, machineStack
		emit((intptr_t)esi);
		//emit("81 EC");				//sub esp, 12
		//emit(12);
#ifdef DEBUG
		emit("81 C4");				//add esp, 4
		emit(4);
#endif
		while(programCounter < romSize)
		{
			Word argument = rom[programCounter].Parameter;
			virtualToNative[programCounter] = nativeRom.size();
			switch(rom[programCounter].Opcode)
			{
			case Opcode::Load:
				emit("83 EE 04"); 								//sub esi, 4
				//Load value at base stack + offset into eax
				//emit("8B 85");								//mov eax, [ebp+offset]
				emit("8B 84 24");								//mov eax, [esp+offset]
				emit((int) (argument.Integer));
				//Store value in eax onto top of program stack
				emit("89 06");									//mov [esi], eax
				break;
			case Opcode::Store:
				//Load value on top of the program stack into eax
				emit("8B 06");									//mov eax, [esi]
				//Load value in eax into memory
				//emit("89 85");								//mov [ebp+offset], eax
				emit("89 84 24");								//mov [esp+offset], eax
				emit((int) (argument.Integer));
				emit("83 C6 04");								//add esi, 4
				break;
			case Opcode::Constant:
				emit("83 EE 04");								//sub esi, 4
				emit("C7 06");									//mov [esi], dword ptr constant
				emit((int) (argument.Integer));
				break;
			case Opcode::Increment:
				if(argument.Integer > 0) {
					emit("81 06");								//add [esi], dword ptr constant
					emit((int) (argument.Integer));
				} else {
					emit("81 2E");								//sub [esi], dword ptr constant
					emit((int) (-argument.Integer));
				}
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
				emitCall((intptr_t)sim, (intptr_t)((void*)&Simulation::create_part));
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
					intptr_t partsArray = (intptr_t)sim->pmap;
					emit("8B 06");								//mov eax, [esi]
					emit("8B 4E 04");							//mov ecx, [esi+4]

					emit("81 F9");								//cmp ecx, XRES
					emit((int)XRES);
					emit("7D 22");						//|--<	//jge 34
					emit("74 20");						//|--<	//jz 32
					emit("3D");							//|		//cmp eax, YRES
					emit((int)YRES);					//|
					emit("7D 19");						//|--<	//jge 25
					emit("74 17");						//|--<	//jz 23
														//|
					emit("69 C0");						//|		//imul eax, 612
					emit((int)XRES);					//|
					emit("01 C8");						//|		//add eax, ecx
					emit("8B 04 85");					//|		//mov eax, [eax*4+pmap]
					emit((int)partsArray);				//|
					emit("C1 F8 08");					//|		//sar eax, 8
					emit("89 46 04");					//|		//mov [esi+4], eax	#Copy eax onto stack
					emit("EB 07");						//| |-<	//jmp +7
					emit("C7 46 04 FF FF FF FF");		//L-+->	//mov [esi+4], -1
					emit("83 C6 04"); 					//  L->	//add esi, 4
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
				{
					intptr_t partsArray = (intptr_t)sim->parts;
					emit("8B 06");								//mov eax, [esi]	#Load index from stack

					emit("3D");									//cmp eax, NPART
					emit((int)NPART);
					emit("7D 23");						//|--<	//jge 31 
					emit("74 21");						//|--<	//jz 29
														//|
					emit("C1 E0 03");					//|		//sal eax, 3		#Shift index left (multiply by 8)	
					emit("8D 14 C5 00 00 00 00");		//|		//lea edx, [eax*8]	#Mutiply by 8 again and copy into edx		//Size of 56 is represented by (1*(8^2))-(1*8)
					emit("89 D1");						//|		//mov ecx, edx		
					emit("29 C1");						//|		//sub ecx, eax		#Subtract index*8^2 by index*8 to get the index*56
					//emit("8B 81");					//|		//mov eax, [ecx+xOffset+partsArray]	#Copy value at index+baseAddress+propertyOffset into eax
					emit("D9 81");						//|		//fld [ecx+xOffset+partsArray]
					emit(partsArray+offsetof(Particle, x));
					emit("DB 1E");						//|		//fistp [esi]
					//emit("89 06");					//|		//mov [esi], eax	#Copy eax onto stack
					//emit("89 81");					//|		//mov eax, [ecx+yOffset+partsArray]	#Copy value at index+baseAddress+propertyOffset into eax
					emit("D9 81");						//|		//fld [ecx+xOffset+partsArray]
					emit(partsArray+offsetof(Particle, y));
					emit("DB 5E FC");					//|		//fistp [esi-4], eax	#Copy eax onto stack
					//emit("89 46 FC");					//|		//mov [esi-4], eax	#Copy eax onto stack
					emit("EB 0D");						//| |-<	//jmp +13
					emit("C7 06 FF FF FF FF");			//L-+-> //mov [esi], -1
					emit("C7 46 FC FF FF FF FF");		//  |	//mov [esi-4] -1
					emit("83 EE 04"); 					//  L->	//sub esi, 4
				}
				break;
			case Opcode::Kill:
				//sim->kill_part(PSPop().Integer);
				//PSPush((Word)0);
				break;
			case Opcode::LoadProperty:
				{
					intptr_t propertyOffset = argument.Integer;
					intptr_t partsArray = (intptr_t)sim->parts;
					emit("8B 06");								//mov eax, [esi]	#Load index from stack
					emit("C1 E0 03");							//sal eax, 3		#Shift index left (multiply by 8)	
					emit("8D 14 C5 00 00 00 00");				//lea edx, [eax*8]	#Mutiply by 8 again and copy into edx		//Size of 56 is represented by (1*(8^2))-(1*8)
					emit("89 D1");								//mov ecx, edx		
					emit("29 C1");								//sub ecx, eax		#Subtract index*8^2 by index*8 to get the index*56
					emit("8B 81");								//mov eax, [ecx+propertyOffset+partsArray]	#Copy value at index+baseAddress+propertyOffset into eax
					emit(partsArray+propertyOffset);
					emit("89 06");								//mov [esi], eax	#Copy eax onto stack
				}
				break;
			case Opcode::StoreProperty:
				{
					intptr_t propertyOffset = argument.Integer;
					intptr_t partsArray = (intptr_t)sim->parts;
					emit("8B 06");								//mov eax, [esi]
					emit("C1 E0 03");							//sal eax, 3
					emit("8D 14 C5 00 00 00 00");				//lea edx, [eax*8]
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
				emit("75 05");									//jne 8
				emit("E9"); 									//jmp [0x12345678]
				emitPlaceholder(argument.Integer);
				break;
			case Opcode::JumpNotEqual:
				emit("83 C6 04"); 								//add esi, 8
				emit("8B 46 FC");								//mov eax, [esi-4]
				emit("3B 46 F8"); 								//cmp eax, [esi-8]
				emit("74 05");									//je 8
				emit("E9"); 									//jmp [0x12345678]
				emitPlaceholder(argument.Integer);
				break;
			case Opcode::JumpGreater:
				emit("83 C6 08");								//add esi 8
				emit("8B 46 FC"); 								//mov eax, [esi-4]
				emit("3B 46 F8"); 								//cmp eax, [esi-8]
				emit("7E 05");									//jng 8
				emit("E9");										//jmp [0x12345678]
				emitPlaceholder(argument.Integer);
				break;
			case Opcode::JumpGreaterEqual:
				emit("83 C6 08");								//add esi 8
				emit("8B 46 FC");								//mov eax, [esi-4]
				emit("3B 46 F8");								//cmp eax, [esi-8]
				emit("7C 05");									//jnge +6
				emit("E9");										//jmp [0x12345678]
				emitPlaceholder(argument.Integer);
				break;
			case Opcode::JumpLess:
				emit("83 C6 08");								//add esi 8
				emit("8B 46 FC");								//mov eax, [esi-4]
				emit("3B 46 F8");								//cmp eax, [esi-8]
				emit("7D 05");									//jnl +6
				emit("E9"); 									//jmp [0x12345678]
				emitPlaceholder(argument.Integer);
				break;
			case Opcode::JumpLessEqual:
				emit("83 C6 08"); 								//add esi 8
				emit("8B 46 FC"); 								//mov eax, [esi-4]
				emit("3B 46 F8"); 								//cmp eax, [esi-8]
				emit("7F 08"); 									//jnle +6
				emit("E9");										//jmp [0x12345678]
				emitPlaceholder(argument.Integer);
				break;
			case Opcode::Jump:
				emit("E9");										//jmp [0x12345678]
				emitPlaceholder(argument.Integer);
				break;
			case Opcode::Return:
				emit("C3");										//ret
				break;
			case Opcode::Leave:
				//emit("81 C7");								//add edi, constant
				emit("81 C4");									//add esp, constant
				emit(argument.Integer);
				break;
			case Opcode::LocalEnter:
				//emit("81 EF");								//sub edi constant
				emit("81 EC");									//sub esp constant
				emit(argument.Integer);
				break;
			}
			//std::cout << programStack << std::endl;
			programCounter++;
		}
#ifdef DEBUG
		emit("81 EC");				//sub esp, 4
		//emit("81 EC");			//sub esp, 4
		emit(4);
		emit("C9");					//leave			//When -fomit-frame-pointers is used, don't 'leave', since ebp isn't on the stack
#endif
		for(std::map<int, int>::iterator iter = placeholders.begin(), end = placeholders.end(); iter != end; ++iter)
		{
			std::pair<int, int> placeholder = *iter;
			int newNativeAddress = virtualToNative[placeholder.second]-placeholder.first-4;
			nativeRom[placeholder.first] = newNativeAddress & 0xFF;
			nativeRom[placeholder.first+1] = (newNativeAddress >> 8) & 0xFF;
			nativeRom[placeholder.first+2] = (newNativeAddress >> 16) & 0xFF;
			nativeRom[placeholder.first+3] = (newNativeAddress >> 24) & 0xFF;
		}
		//std::cout << "CS: " << callStack << " PS: " << programStack << std::endl;
		return nativeRom;
	#endif
	}

	void X86Native::emitPlaceholder(int virtualAddress)
	{
		placeholders[nativeRom.size()] = virtualAddress;
		emit((int)0);
	}

	void X86Native::emitCall(intptr_t objectPtr, intptr_t functionAddress)
	{
		emit("B9");												//mov ecx, instancePointer
		emit((int) objectPtr);	
		emit("A1");												//mov eax, functionAddress
		emit((int)functionAddress);
		emit("FF D0");											//call eax		
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