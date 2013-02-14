#include "Opcodes.h"
#include "X86Native.h"
#include "simulation/Simulation.h"

namespace pim 
{
	std::vector<unsigned char> X86Native::Compile(Simulation * sim, unsigned char * machineStack, unsigned char * heap, Instruction * rom, int romSize)
	{
	#if defined(X86) && !defined(_64BIT)
		int programCounter = 0;
		nativeRom.clear();

		for(int i = 0; i < 8; i++) { emit("90"); } 				//nop, helps find the code in memory with a debugger
		
		//emit("BE");												//mov esi, machineStack
		//emitConstantP((intptr_t)machineStack);

		while(programCounter < romSize)
		{
			Word argument = rom[programCounter].Parameter;
			virtualToNative[programCounter] = nativeRom.size();
			switch(rom[programCounter].Opcode)
			{
			case Opcode::Begin:
				emit("BE");										//mov esi, machineStack
				emitConstantP((intptr_t)machineStack);
				break;
			case Opcode::Load:
				emit("83 EE 04"); 								//sub esi, 4
				//Load value at base stack + offset into eax
				//emit("8B 85");								//mov eax, [ebp+offset]
				emit("8B 84 24");								//mov eax, [esp+offset]
				emitConstantP(argument.Integer);
				//Store value in eax onto top of program stack
				emit("89 06");									//mov [esi], eax
				break;
			case Opcode::Store:
				//Load value on top of the program stack into eax
				emit("8B 06");									//mov eax, [esi]
				//Load value in eax into memory
				emit("89 84 24");								//mov [esp+offset], eax
				emitConstantD(argument.Integer);
				emit("83 C6 04");								//add esi, 4
				break;
			case Opcode::LoadAdr:
				emit("83 EE 04"); 								//sub esi, 4
				emit("A1");										//mov eax, [offset]
				emitConstantP(((intptr_t)heap)+argument.Integer);
				emit("89 06");									//mov [esi], eax
				break;
			case Opcode::StoreAdr:
				emit("8B 06");									//mov eax, [esi]
				emit("A3");										//mov [offset], eax
				emitConstantP(((intptr_t)heap)+argument.Integer);
				emit("83 C6 04");								//add esi, 4
				break;
			case Opcode::Constant:
				emit("83 EE 04");								//sub esi, 4
				emit("C7 06");									//mov [esi], dword ptr constant
				emitConstantD(argument.Integer);
				break;
			case Opcode::Increment:
				if(argument.Integer > 0) {
					emit("81 06");								//add [esi], dword ptr constant
					emitConstantD(argument.Integer);
				} else {
					emit("81 2E");								//sub [esi], dword ptr constant
					emitConstantD(-argument.Integer);
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
				emit("8B 06");									//mov eax, [esi]
				emit("50");										//push eax
				emit("8B 46 04");								//mov eax, [esi+4]
				emit("50");										//push eax
				emit("8B 46 08");								//mov eax, [esi+8]
				emit("50");										//push eax
				emit("8B 46 0C");								//mov eax, [esi+12]
				emit("50");										//push eax
				emit("83 C6 08");								//add esi, 8
				emitCall((intptr_t)sim, (intptr_t)((void*)&Simulation::create_part), 16);
				emit("89 06");									//mov [esi], eax
				break;
			case Opcode::Transform:
				//PSPop();
				//PSPop();
				//PSPush((Word)-1);
				break;
			case Opcode::Get:
				{
					intptr_t partsArray = (intptr_t)sim->pmap;
					emit("8B 06");								//mov eax, [esi]
					emit("8B 4E 04");							//mov ecx, [esi+4]
					emit("81 F9");								//cmp ecx, XRES		//Push -1 if y is out of range
					emitConstantD((int)XRES);
					emit("7D 2C");						//|--<	//jge 34
					emit("83 F9 00");					//|		//cmp ecx, 0
					emit("78 27");						//|--<	//js 32
					emit("3D");							//|		//cmp eax, YRES		//Push -1 if x is out of range
					emitConstantD((int)YRES);			//|
					emit("7D 20");						//|--<	//jge 25
					emit("83 F8 00");					//|		//cmp eax, 0
					emit("78 1B");						//|--<	//js 23
					emit("69 C0");						//|		//imul eax, 612
					emitConstantD((int)XRES);			//|
					emit("01 C8");						//|		//add eax, ecx
					emit("8B 04 85");					//|		//mov eax, [eax*4+pmap]
					emitConstantP((int)partsArray);		//|
					emit("85 C0");						//|		//test eax, eax		//Push 0 if location if empty
					emit("74 08");						//|	|-<	//je +5
					emit("C1 F8 08");					//|	|	//sar eax, 8
					emit("89 46 04");					//|	|	//mov [esi+4], eax	#Copy eax onto stack
					emit("EB 07");						//| |-<	//jmp +7
					emit("C7 46 04 FF FF FF FF");		//L-+->	//mov [esi+4], -1
					emit("83 C6 04"); 					//  L->	//add esi, 4
				}
				break;
			case Opcode::Position:
				{
					intptr_t partsArray = (intptr_t)sim->parts;
					emit("8B 06");								//mov eax, [esi]	#Load index from stack
					emit("3D");									//cmp eax, NPART
					emitConstantD((int)NPART);
					emit("7D 26");						//|--<	//jge 31 
					emit("83 F8 00");					//|		//cmp eax, 0
					emit("78 21");						//|--<	//js 29
					emit("C1 E0 03");					//|		//sal eax, 3		#Shift index left (multiply by 8)	
					emit("8D 14 C5 00 00 00 00");		//|		//lea edx, [eax*8]	#Mutiply by 8 again and copy into edx		//Size of 56 is represented by (1*(8^2))-(1*8)
					emit("89 D1");						//|		//mov ecx, edx		
					emit("29 C1");						//|		//sub ecx, eax		#Subtract index*8^2 by index*8 to get the index*56
					emit("D9 81");						//|		//fld [ecx+xOffset+partsArray]
					emitConstantP(partsArray+offsetof(Particle, x));
					emit("DB 1E");						//|		//fistp [esi]
					emit("D9 81");						//|		//fld [ecx+xOffset+partsArray]
					emitConstantP(partsArray+offsetof(Particle, y));
					emit("DB 5E FC");					//|		//fistp [esi-4], eax	#Copy eax onto stack
					emit("EB 0D");						//| |-<	//jmp +13
					emit("C7 06 FF FF FF FF");			//L-+-> //mov [esi], -1
					emit("C7 46 FC FF FF FF FF");		//  |	//mov [esi-4] -1
					emit("83 EE 04"); 					//  L->	//sub esi, 4
				}
				break;
			case Opcode::Kill:
				emit("8B 06");									//mov eax, [esi]
				emit("50");										//push eax
				emitCall((intptr_t)sim, (intptr_t)((void*)&Simulation::kill_part), 4);
				emit("C7 06 00 00 00 00");						//mov [esi], eax
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
					emitConstantP(partsArray+propertyOffset);
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
					emitConstantP(partsArray+propertyOffset);
					emit("83 C6 08"); 							//add esi, 8
				}
				break;
			case Opcode::ToFloat:
				emit("DB 06");									//fild [esi]
				emit("D9 1E");									//fstp [esi]
				break;
			case Opcode::ToInteger:
				emit("D9 06");									//fld [esi]
				emit("DB 1E");									//fistp [esi]
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
				emit("83 C6 08"); 								//add esi, 8
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
				emit("81 C4");									//add esp, constant
				emitConstantD(argument.Integer);
				break;
			case Opcode::LocalEnter:
				emit("81 EC");									//sub esp constant
				emitConstantD(argument.Integer);
				break;
			}
			//std::cout << programStack << std::endl;
			programCounter++;
		}

		for(int i = 0; i < 8; i++) { emit("90"); } 				//nop, helps find the code in memory with a debugger

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
		emitConstantD((int)0);
	}

	void X86Native::emitCall(intptr_t objectPtr, intptr_t functionAddress, int stackSize)
	{
#ifdef _MSC_VER
		//MSVC puts the instance pointer in ecx, not on the stack
		emit("B9");												//mov ecx, instancePointer
		emitConstantP(objectPtr);	
		emit("B8");												//mov eax, functionAddress
		emitConstantP(functionAddress);
		emit("FF D0");											//call eax
		emit("81 C4");											//add esp, stacksize
		emitConstantD(stackSize);
#else
		emit("68");												//push instancePointer
		emitConstantP(objectPtr);	
		emit("B8");												//mov eax, functionAddress
		emitConstantP(functionAddress);
		emit("FF D0");											//call eax
		emit("81 C4");											//add esp, stacksize
		emitConstantD(stackSize+sizeof(intptr_t));
#endif
	}

	void X86Native::emitConstantD(long int constant)
	{
		nativeRom.push_back(constant & 0xFF);
		nativeRom.push_back((constant >> 8) & 0xFF);
		nativeRom.push_back((constant >> 16) & 0xFF);
		nativeRom.push_back((constant >> 24) & 0xFF);
	}

	void X86Native::emitConstantP(intptr_t constant)
	{
		emitConstantD(constant);
	}

}