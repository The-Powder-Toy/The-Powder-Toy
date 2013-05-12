#ifdef VMJIT

#include <cstdio>
#include "VirtualMachine.h"

#ifdef WIN32
#include "Windows.h"
#endif

namespace vm
{
	#define OP(n) OP##n
	/*

	  eax	scratch
	  ebx	scratch
	  ecx	scratch (required for shifts)
	  edx	scratch (required for divisions)
	  esi	RP
	  edi	DP

	*/

	// TTimo: initialised the statics, this fixes a crash when entering a compiled VM 
	static unsigned char		*buf = NULL;
	static unsigned char		*jused = NULL;
	static int					compiledOfs = 0;
	static int					pc = 0;

	//static int					callMask = 0; // bk001213 - init
	static int					eDP;
	static int					eRP;
	static int					instruction, pass;
	static int					lastConst = 0;
	static int					oc0, oc1, pop0, pop1;

	static int					eRamMask = 0;
	static int					eRomMask = 0;
	static int					* eInstructionPointers = NULL;

	static int					eSyscallNum;
	static void					* eRam = NULL;
	static VirtualMachine		* eVM = NULL;

	static int callFromCompiledPtr = (int)VirtualMachine::callFromCompiled;
	static int callSyscallPtr = (int)VirtualMachine::callSyscall;

	typedef enum 
	{
		LAST_COMMAND_NONE	= 0,
		LAST_COMMAND_MOV_EDI_EAX,
		LAST_COMMAND_SUB_DI_4,
		LAST_COMMAND_SUB_DI_8,
	} ELastCommand;

	static ELastCommand LastCommand;

	void VirtualMachine::callSyscall()
	{
		//throw RuntimeException("Turd");

		/*VirtualMachine * savedVM;
		int * callOpStack2;

		savedVM = eVM;
		callOpStack2 = (int*)eOpStack;

		// save the stack to allow recursive VM entry
		eVM->DP = eDP - 4;
		*(int *)((byte *)eVM->ram + eDP + 4) = eSyscallNum;
	//VM_LogSyscalls(  (int *)((byte *)currentVM->dataBase + programStack + 4) );
		*(callOpStack2+1) = eVM->syscall( *(int *)((unsigned char *)eVM->ram + eDP + 4) );

	 	eVM = savedVM;*/
	}

	void VirtualMachine::callFromCompiled()
	{
		/*__asm__("doAsmCall:      				\n\t" \
				"	movl (%%edi),%%eax			\n\t" \
				"	subl $4,%%edi				\n\t" \
				"   orl %%eax,%%eax				\n\t" \
				"	jl systemCall				\n\t" \
				"	shll $2,%%eax				\n\t" \
				"	addl %3,%%eax				\n\t" \
				"	call *(%%eax)				\n\t" \
				"   movl (%%edi),%%eax			\n\t" \
				"   andl %5, %%eax				\n\t" \
				"	jmp doret					\n\t" \
				"systemCall:					\n\t" \
				"	negl %%eax					\n\t" \
				"	decl %%eax					\n\t" \
				"	movl %%eax,%0				\n\t" \
				"	movl %%esi,%1				\n\t" \
				"	movl %%edi,%2				\n\t" \
				"	pushl %%ecx					\n\t" \
				"	pushl %%esi					\n\t" \
				"	pushl %%edi					\n\t" \
				"	call *%4						\n\t" \
				"	popl %%edi					\n\t" \
				"	popl %%esi					\n\t" \
				"	popl %%ecx					\n\t" \
				"	addl $4,%%edi				\n\t" \
				"doret:							\n\t" \
				"	ret							\n\t" \
				: "=rm" (eSyscallNum), "=rm" (eDP), "=rm" (eOpStack) \
				: "rm" (eInstructionPointers), "r" (callSyscall), "m" (eRomMask) \
				: "ax", "di", "si", "cx" \
		);*/
				//"	call *%4						\n\t"

				//"	negl %%eax					\n\t"
			//	"	decl %%eax					\n\t"
		__asm__ volatile ("doAsmCall:      				\n\t" \
				"	movl (%%edi),%%eax			\n\t" \
				"	subl $4,%%edi				\n\t" \
				"   orl %%eax,%%eax				\n\t" \
				"	jl systemCall				\n\t" \
				"	shll $2,%%eax				\n\t" \
				"	addl %3,%%eax				\n\t" \
				"	call *(%%eax)				\n\t" \
				"   movl (%%edi),%%eax			\n\t" \
				"   andl %5, %%eax				\n\t" \
				"	ret							\n\t" \
				"systemCall:					\n\t" \
				"	movl %%eax,%0				\n\t" \
				"	movl %%esi,%1				\n\t" \
				"	movl %%edi,%2				\n\t" \
				"	pushl %%ecx					\n\t" \
				"	pushl %%esi					\n\t" \
				"	pushl %%edi					\n\t" \
				: "=rm" (eSyscallNum), "=rm" (eRP), "=rm" (eDP) \
				: "rm" (eInstructionPointers), "r" (callSyscall), "m" (eRomMask) \
				: "ax", "di", "si", "cx" \
		);
				//printf("Syscall: %d\n", eSyscallNum);
				//throw RuntimeException("Turd");

				eVM->DP = eDP-((int)eRam);
				eVM->syscall(eSyscallNum);
				eDP = eVM->DP+((int)eRam)-4;

				//"	addl $4,%edi				\n\t"
	//			"	ret							\n\t"
		__asm__ volatile ("popl %edi					\n\t" \
				"	popl %esi					\n\t" \
				"	popl %ecx					\n\t" \
				"doret:							\n\t" \
		);
	}

	int VirtualMachine::CallCompiled(int address)
	{
		//int		stack[1024];
		//int stack[100];
		int programStack;
		int stackOnEntry;
		unsigned char * image;
		void * entryPoint;
		int * oldInstructionPointers;
		void * oldDataStack;

		oldDataStack = eRam;
		oldInstructionPointers = eInstructionPointers;

		eVM = this;
		eInstructionPointers = instructionPointers;

		eRomMask = romMask;
		eRamMask = ramMask;

		// we might be called recursively, so this might not be the very top
		eDP =  ((int)ram)+DP;
		stackOnEntry = DP;

		// set up the stack frame 
		image = (unsigned char *)ram;//vm->dataBase;

		eDP -= 48;

		//*(int *)&image[ programStack + 44] = args[9];
		//*(int *)&image[ programStack + 40] = args[8];
		//*(int *)&image[ programStack + 36] = args[7];
		//*(int *)&image[ programStack + 32] = args[6];
		//*(int *)&image[ programStack + 28] = args[5];
		//*(int *)&image[ programStack + 24] = args[4];
		//*(int *)&image[ programStack + 20] = args[3];
		//*(int *)&image[ programStack + 16] = args[2];
		//*(int *)&image[ programStack + 12] = args[1];
		//*(int *)&image[ programStack + 8 ] = args[0];
		//*(int *)&image[ programStack + 4 ] = 0;	// return stack
		//*(int *)&image[ programStack ] = -1;	// will terminate the loop on return

		// off we go into generated code...
		entryPoint = compiledRom;//0;//vm->codeBase;
		eRam = ram+dataStack;

	#if defined(_MSC_VER)
		__asm  {
			pushad
			mov		esi, DP;
			mov		edi, opStack
			call	entryPoint
			mov		DP, esi
			mov		opStack, edi
			popad
		}
	#else
		{
			static int memDP;
			static int memRP;
			static void *memEntryPoint;

			memDP = DP+((int)ram);
			memRP = RP;
			memEntryPoint = entryPoint;
			
			__asm__("	pushal				\r\n" \
					"	movl %0,%%esi		\r\n" \
					"	movl %1,%%edi		\r\n" \
					"	call *%2			\r\n" \
					"	movl %%esi,%0		\r\n" \
					"	movl %%edi,%1		\r\n" \
					"	popal				\r\n" \
					: "=m" (memRP), "=m" (memDP) \
					: "m" (memEntryPoint), "0" (memRP), "1" (memDP) \
					: "si", "di" \
			);

			DP = memDP-((int)ram);
			RP = memRP;
		}
	#endif

		if ( eRam != ram+dataStack ) {
			throw RuntimeException("opStack corrupted in compiled code");
		}
		if ( DP != stackOnEntry+4 ) {
			printf("DP: %d, stackOnEntry: %d\n", DP, stackOnEntry);
			throw RuntimeException("programStack corrupted in compiled code");
		}

		DP = stackOnEntry;

		// in case we were recursively called by another vm
		eInstructionPointers = oldInstructionPointers;
		eRam = oldDataStack;

		return 0;//*(int *)eOpStack;
	}

	bool VirtualMachine::Compile()
	{
		Instruction	op;
		int		maxLength;
		int		v;
		int		i;
		bool opt;

		// allocate a very large temp buffer, we will shrink it later
		maxLength = romSize * 8;
		buf = new unsigned char[maxLength];
		jused = new unsigned char[romSize + 2];
		instructionPointers = new int[romSize];
		std::fill(jused, jused+romSize+2, 0);

		for(pass=0; pass<2; pass++) {
			oc0 = -23423;
			oc1 = -234354;
			pop0 = -43435;
			pop1 = -545455;

			// translate all instructions
			pc = 0;
			instruction = 0;
			compiledOfs = 0;

			LastCommand = LAST_COMMAND_NONE;

			while (instruction < romSize)
			{
				if (compiledOfs > maxLength - 16)
				{
					throw JITException("Compile: maxLength exceeded");
				}

				instructionPointers[instruction] = compiledOfs;
				instruction++;

				if (pc > romSize)
				{
					throw JITException("Compile: program counter run off the edge");
				}

				op = rom[pc];
				pc++;
				switch ( op.Operation )
				{
				case 0:
					break;
				case OP(BREAK):
					emitInstruction( "CC" );			// int 3
					break;
				case OP(ENTER):
					//emitInstruction( "CC" );			// int 3
					emitInstruction( "81 EE" );		// sub	esi, 0x12345678
					emit4( constant4() );
					break;
				case OP(CONST):
					if (rom[pc].Operation == OP(LOAD4))
					{
						emitAddEDI4();
						emitInstruction( "BB" );		// mov	ebx, 0x12345678
						emit4( (constant4()&ramMask) + (int)ram);
						emitInstruction( "8B 03" );		// mov	eax, dword ptr [ebx]
						emitCommand(LAST_COMMAND_MOV_EDI_EAX);		// mov dword ptr [edi], eax
						pc++;						// OP(LOAD4)
						instruction += 1;
						break;
					}
					if (rom[pc].Operation == OP(LOAD2))
					{
						emitAddEDI4();
						emitInstruction( "BB" );		// mov	ebx, 0x12345678
						emit4( (constant4()&ramMask) + (int)ram);
						emitInstruction( "0F B7 03" );	// movzx	eax, word ptr [ebx]
						emitCommand(LAST_COMMAND_MOV_EDI_EAX);		// mov dword ptr [edi], eax
						pc++;						// OP(LOAD4)
						instruction += 1;
						break;
					}
					if (rom[pc].Operation == OP(LOAD1))
					{
						emitAddEDI4();
						emitInstruction( "BB" );		// mov	ebx, 0x12345678
						emit4( (constant4()&ramMask) + (int)ram);
						emitInstruction( "0F B6 03" );	// movzx	eax, byte ptr [ebx]
						emitCommand(LAST_COMMAND_MOV_EDI_EAX);		// mov dword ptr [edi], eax
						pc++;						// OP(LOAD4)
						instruction += 1;
						break;
					}
					if (rom[pc].Operation == OP(STORE4))
					{
						opt = emitMovEBXEDI((ramMask & ~3));
						emitInstruction( "B8" );			// mov	eax, 0x12345678
						emit4( constant4() );
		//				if (!opt) {
		//					emitInstruction( "81 E3" );		// and ebx, 0x12345678
		//					emit4( ramMask & ~3 );
		//				}
						emitInstruction( "89 83" );		// mov dword ptr [ebx+0x12345678], eax
						emit4( (int)ram );
						emitCommand(LAST_COMMAND_SUB_DI_4);		// sub edi, 4
						pc++;						// OP(STORE4)
						instruction += 1;
						break;
					}
					if (rom[pc].Operation == OP(STORE2))
					{
						opt = emitMovEBXEDI((ramMask & ~1));
						emitInstruction( "B8" );			// mov	eax, 0x12345678
						emit4( constant4() );
		//				if (!opt) {
		//					emitInstruction( "81 E3" );		// and ebx, 0x12345678
		//					emit4( ramMask & ~1 );
		//				}
						emitInstruction( "66 89 83" );	// mov word ptr [ebx+0x12345678], eax
						emit4( (int)ram );
						emitCommand(LAST_COMMAND_SUB_DI_4);		// sub edi, 4
						pc++;						// OP(STORE4)
						instruction += 1;
						break;
					}
					if (rom[pc].Operation == OP(STORE1))
					{
						opt = emitMovEBXEDI(ramMask);
						emitInstruction( "B8" );			// mov	eax, 0x12345678
						emit4( constant4() );
		//				if (!opt) {
		//					emitInstruction( "81 E3" );	// and ebx, 0x12345678
		//					emit4( ramMask );
		//				}
						emitInstruction( "88 83" );		// mov byte ptr [ebx+0x12345678], eax
						emit4( (int)ram );
						emitCommand(LAST_COMMAND_SUB_DI_4);		// sub edi, 4
						pc++;						// OP(STORE4)
						instruction += 1;
						break;
					}
					if (rom[pc].Operation == OP(ADD))
					{
						emitInstruction( "81 07" );		// add dword ptr [edi], 0x1234567
						emit4( constant4() );
						pc++;						// OP(ADD)
						instruction += 1;
						break;
					}
					if (rom[pc].Operation == OP(SUB))
					{
						emitInstruction( "81 2F" );		// sub dword ptr [edi], 0x1234567
						emit4( constant4() );
						pc++;						// OP(ADD)
						instruction += 1;
						break;
					}
					emitAddEDI4();
					emitInstruction( "C7 07" );		// mov	dword ptr [edi], 0x12345678
					lastConst = constant4();
					emit4( lastConst );
					if (rom[pc].Operation == OP(JUMP))
					{
						jused[lastConst] = 1;
					}
					break;
				case OP(LOCAL):
					emitAddEDI4();
					emitInstruction( "8D 86" );		// lea eax, [0x12345678 + esi]
					oc0 = oc1;
					oc1 = constant4();
					emit4( oc1 );
					emitCommand(LAST_COMMAND_MOV_EDI_EAX);		// mov dword ptr [edi], eax
					break;
				case OP(ARG):
					emitMovEAXEDI();			// mov	eax,dword ptr [edi]
					emitInstruction( "89 86" );		// mov	dword ptr [esi+ram],eax
					// FIXME: range check
					emit4( constant1() + (int)ram );
					emitCommand(LAST_COMMAND_SUB_DI_4);		// sub edi, 4
					break;
				case OP(CALL):
					emitInstruction("C7 86");		// mov dword ptr [esi+ram],0x12345678
					emit4((int)ram);
					emit4(pc);
					emitInstruction("FF 15");		// call callFromCompiled
					emit4((int)&callFromCompiledPtr);
					break;
				case OP(PUSH):
					emitAddEDI4();
					break;
				case OP(POP):
					emitCommand(LAST_COMMAND_SUB_DI_4);		// sub edi, 4
					break;
				case OP(LEAVE):
					v = constant4();
					emitInstruction( "81 C6" );		// add	esi, 0x12345678
					emit4( v );
					emitInstruction( "C3" );			// ret
					break;
				case OP(LOAD4):
					if (rom[pc].Operation == OP(CONST) && rom[pc+1].Operation == OP(ADD) && rom[pc+2].Operation == OP(STORE4))
					{
						if (oc0 == oc1 && pop0 == OP(LOCAL) && pop1 == OP(LOCAL))
						{
							compiledOfs -= 11;
							instructionPointers[ instruction-1 ] = compiledOfs;
						}
						pc++;						// OP(CONST)
						v = constant4();
						emitMovEBXEDI(ramMask);
						if (v == 1 && oc0 == oc1 && pop0 == OP(LOCAL) && pop1 == OP(LOCAL))
						{
							emitInstruction( "FF 83");		// inc dword ptr [ebx + 0x12345678]
							emit4( (int)ram );
						}
						else
						{
							emitInstruction( "8B 83" );		// mov	eax, dword ptr [ebx + 0x12345678]
							emit4( (int)ram );
							emitInstruction( "05"  );		// add eax, const
							emit4( v );
							if (oc0 == oc1 && pop0 == OP(LOCAL) && pop1 == OP(LOCAL))
							{
								emitInstruction( "89 83" );		// mov dword ptr [ebx+0x12345678], eax
								emit4( (int)ram );
							}
							else
							{
								emitCommand(LAST_COMMAND_SUB_DI_4);		// sub edi, 4
								emitInstruction( "8B 1F" );		// mov	ebx, dword ptr [edi]
								emitInstruction( "89 83" );		// mov dword ptr [ebx+0x12345678], eax
								emit4( (int)ram );
							}
						}
						emitCommand(LAST_COMMAND_SUB_DI_4);		// sub edi, 4
						pc++;						// OP(ADD)
						pc++;						// OP(STORE)
						instruction += 3;
						break;
					}

					if (rom[pc].Operation == OP(CONST) && rom[pc+1].Operation == OP(SUB) && rom[pc+2].Operation == OP(STORE4))
					{
						if (oc0 == oc1 && pop0 == OP(LOCAL) && pop1 == OP(LOCAL)) {
							compiledOfs -= 11;
							instructionPointers[ instruction-1 ] = compiledOfs;
						}
						emitMovEBXEDI(ramMask);
						emitInstruction( "8B 83" );		// mov	eax, dword ptr [ebx + 0x12345678]
						emit4( (int)ram );
						pc++;						// OP(CONST)
						v = constant4();
						if (v == 1 && oc0 == oc1 && pop0 == OP(LOCAL) && pop1 == OP(LOCAL))
						{
							emitInstruction( "FF 8B");		// dec dword ptr [ebx + 0x12345678]
							emit4( (int)ram );
						}
						else
						{
							emitInstruction( "2D"  );		// sub eax, const
							emit4( v );
							if (oc0 == oc1 && pop0 == OP(LOCAL) && pop1 == OP(LOCAL))
							{
								emitInstruction( "89 83" );		// mov dword ptr [ebx+0x12345678], eax
								emit4( (int)ram );
							}
							else
							{
								emitCommand(LAST_COMMAND_SUB_DI_4);		// sub edi, 4
								emitInstruction( "8B 1F" );		// mov	ebx, dword ptr [edi]
								emitInstruction( "89 83" );		// mov dword ptr [ebx+0x12345678], eax
								emit4( (int)ram );
							}
						}
						emitCommand(LAST_COMMAND_SUB_DI_4);		// sub edi, 4
						pc++;						// OP(SUB)
						pc++;						// OP(STORE)
						instruction += 3;
						break;
					}

					if (buf[compiledOfs-2] == 0x89 && buf[compiledOfs-1] == 0x07)
					{
						compiledOfs -= 2;
						instructionPointers[ instruction-1 ] = compiledOfs;
						emitInstruction( "8B 80");	// mov eax, dword ptr [eax + 0x1234567]
						emit4( (int)ram );
						emitCommand(LAST_COMMAND_MOV_EDI_EAX);		// mov dword ptr [edi], eax
						break;
					}
					emitMovEBXEDI(ramMask);
					emitInstruction( "8B 83" );		// mov	eax, dword ptr [ebx + 0x12345678]
					emit4( (int)ram );
					emitCommand(LAST_COMMAND_MOV_EDI_EAX);		// mov dword ptr [edi], eax
					break;
				case OP(LOAD2):
					emitMovEBXEDI(ramMask);
					emitInstruction( "0F B7 83" );	// movzx	eax, word ptr [ebx + 0x12345678]
					emit4( (int)ram );
					emitCommand(LAST_COMMAND_MOV_EDI_EAX);		// mov dword ptr [edi], eax
					break;
				case OP(LOAD1):
					emitMovEBXEDI(ramMask);
					emitInstruction( "0F B6 83" );	// movzx eax, byte ptr [ebx + 0x12345678]
					emit4( (int)ram );
					emitCommand(LAST_COMMAND_MOV_EDI_EAX);		// mov dword ptr [edi], eax
					break;
				case OP(STORE4):
					emitMovEAXEDI();	
					emitInstruction( "8B 5F FC" );	// mov	ebx, dword ptr [edi-4]
		//			if (pop1 != OP(CALL)) {
		//				emitInstruction( "81 E3" );		// and ebx, 0x12345678
		//				emit4( ramMask & ~3 );
		//			}
					emitInstruction( "89 83" );		// mov dword ptr [ebx+0x12345678], eax
					emit4( (int)ram );
					emitCommand(LAST_COMMAND_SUB_DI_8);		// sub edi, 8
					break;
				case OP(STORE2):
					emitMovEAXEDI();	
					emitInstruction( "8B 5F FC" );	// mov	ebx, dword ptr [edi-4]
		//			emitInstruction( "81 E3" );		// and ebx, 0x12345678
		//			emit4( ramMask & ~1 );
					emitInstruction( "66 89 83" );	// mov word ptr [ebx+0x12345678], eax
					emit4( (int)ram );
					emitCommand(LAST_COMMAND_SUB_DI_8);		// sub edi, 8
					break;
				case OP(STORE1):
					emitMovEAXEDI();	
					emitInstruction( "8B 5F FC" );	// mov	ebx, dword ptr [edi-4]
		//			emitInstruction( "81 E3" );		// and ebx, 0x12345678
		//			emit4( ramMask );
					emitInstruction( "88 83" );		// mov byte ptr [ebx+0x12345678], eax
					emit4( (int)ram );
					emitCommand(LAST_COMMAND_SUB_DI_8);		// sub edi, 8
					break;

				case OP(EQ):
					emitCommand(LAST_COMMAND_SUB_DI_8);		// sub edi, 8
					emitInstruction( "8B 47 04" );	// mov	eax, dword ptr [edi+4]
					emitInstruction( "3B 47 08" );	// cmp	eax, dword ptr [edi+8]
					emitInstruction( "75 06" );		// jne +6
					emitInstruction( "FF 25" );		// jmp	[0x12345678]
					v = constant4();
					jused[v] = 1;
					emit4( (int)instructionPointers + v*4 );
					break;
				case OP(NE):
					emitCommand(LAST_COMMAND_SUB_DI_8);		// sub edi, 8
					emitInstruction( "8B 47 04" );	// mov	eax, dword ptr [edi+4]
					emitInstruction( "3B 47 08" );	// cmp	eax, dword ptr [edi+8]
					emitInstruction( "74 06" );		// je +6
					emitInstruction( "FF 25" );		// jmp	[0x12345678]
					v = constant4();
					jused[v] = 1;
					emit4( (int)instructionPointers + v*4 );
					break;
				case OP(LTI):
					emitCommand(LAST_COMMAND_SUB_DI_8);		// sub edi, 8
					emitInstruction( "8B 47 04" );	// mov	eax, dword ptr [edi+4]
					emitInstruction( "3B 47 08" );	// cmp	eax, dword ptr [edi+8]
					emitInstruction( "7D 06" );		// jnl +6
					emitInstruction( "FF 25" );		// jmp	[0x12345678]
					v = constant4();
					jused[v] = 1;
					emit4( (int)instructionPointers + v*4 );
					break;
				case OP(LEI):
					emitCommand(LAST_COMMAND_SUB_DI_8);		// sub edi, 8
					emitInstruction( "8B 47 04" );	// mov	eax, dword ptr [edi+4]
					emitInstruction( "3B 47 08" );	// cmp	eax, dword ptr [edi+8]
					emitInstruction( "7F 06" );		// jnle +6
					emitInstruction( "FF 25" );		// jmp	[0x12345678]
					v = constant4();
					jused[v] = 1;
					emit4( (int)instructionPointers + v*4 );
					break;
				case OP(GTI):
					emitCommand(LAST_COMMAND_SUB_DI_8);		// sub edi, 8
					emitInstruction( "8B 47 04" );	// mov	eax, dword ptr [edi+4]
					emitInstruction( "3B 47 08" );	// cmp	eax, dword ptr [edi+8]
					emitInstruction( "7E 06" );		// jng +6
					emitInstruction( "FF 25" );		// jmp	[0x12345678]
					v = constant4();
					jused[v] = 1;
					emit4( (int)instructionPointers + v*4 );
					break;
				case OP(GEI):
					emitCommand(LAST_COMMAND_SUB_DI_8);		// sub edi, 8
					emitInstruction( "8B 47 04" );	// mov	eax, dword ptr [edi+4]
					emitInstruction( "3B 47 08" );	// cmp	eax, dword ptr [edi+8]
					emitInstruction( "7C 06" );		// jnge +6
					emitInstruction( "FF 25" );		// jmp	[0x12345678]
					v = constant4();
					jused[v] = 1;
					emit4( (int)instructionPointers + v*4 );
					break;
				case OP(LTU):
					emitCommand(LAST_COMMAND_SUB_DI_8);		// sub edi, 8
					emitInstruction( "8B 47 04" );	// mov	eax, dword ptr [edi+4]
					emitInstruction( "3B 47 08" );	// cmp	eax, dword ptr [edi+8]
					emitInstruction( "73 06" );		// jnb +6
					emitInstruction( "FF 25" );		// jmp	[0x12345678]
					v = constant4();
					jused[v] = 1;
					emit4( (int)instructionPointers + v*4 );
					break;
				case OP(LEU):
					emitCommand(LAST_COMMAND_SUB_DI_8);		// sub edi, 8
					emitInstruction( "8B 47 04" );	// mov	eax, dword ptr [edi+4]
					emitInstruction( "3B 47 08" );	// cmp	eax, dword ptr [edi+8]
					emitInstruction( "77 06" );		// jnbe +6
					emitInstruction( "FF 25" );		// jmp	[0x12345678]
					v = constant4();
					jused[v] = 1;
					emit4( (int)instructionPointers + v*4 );
					break;
				case OP(GTU):
					emitCommand(LAST_COMMAND_SUB_DI_8);		// sub edi, 8
					emitInstruction( "8B 47 04" );	// mov	eax, dword ptr [edi+4]
					emitInstruction( "3B 47 08" );	// cmp	eax, dword ptr [edi+8]
					emitInstruction( "76 06" );		// jna +6
					emitInstruction( "FF 25" );		// jmp	[0x12345678]
					v = constant4();
					jused[v] = 1;
					emit4( (int)instructionPointers + v*4 );
					break;
				case OP(GEU):
					emitCommand(LAST_COMMAND_SUB_DI_8);		// sub edi, 8
					emitInstruction( "8B 47 04" );	// mov	eax, dword ptr [edi+4]
					emitInstruction( "3B 47 08" );	// cmp	eax, dword ptr [edi+8]
					emitInstruction( "72 06" );		// jnae +6
					emitInstruction( "FF 25" );		// jmp	[0x12345678]
					v = constant4();
					jused[v] = 1;
					emit4( (int)instructionPointers + v*4 );
					break;
				case OP(EQF):
					emitCommand(LAST_COMMAND_SUB_DI_8);		// sub edi, 8
					emitInstruction( "D9 47 04" );	// fld dword ptr [edi+4]
					emitInstruction( "D8 5F 08" );	// fcomp dword ptr [edi+8]
					emitInstruction( "DF E0" );		// fnstsw ax
					emitInstruction( "F6 C4 40" );	// test	ah,0x40
					emitInstruction( "74 06" );		// je +6
					emitInstruction( "FF 25" );		// jmp	[0x12345678]
					v = constant4();
					jused[v] = 1;
					emit4( (int)instructionPointers + v*4 );
					break;			
				case OP(NEF):
					emitCommand(LAST_COMMAND_SUB_DI_8);		// sub edi, 8
					emitInstruction( "D9 47 04" );	// fld dword ptr [edi+4]
					emitInstruction( "D8 5F 08" );	// fcomp dword ptr [edi+8]
					emitInstruction( "DF E0" );		// fnstsw ax
					emitInstruction( "F6 C4 40" );	// test	ah,0x40
					emitInstruction( "75 06" );		// jne +6
					emitInstruction( "FF 25" );		// jmp	[0x12345678]
					v = constant4();
					jused[v] = 1;
					emit4( (int)instructionPointers + v*4 );
					break;			
				case OP(LTF):
					emitCommand(LAST_COMMAND_SUB_DI_8);		// sub edi, 8
					emitInstruction( "D9 47 04" );	// fld dword ptr [edi+4]
					emitInstruction( "D8 5F 08" );	// fcomp dword ptr [edi+8]
					emitInstruction( "DF E0" );		// fnstsw ax
					emitInstruction( "F6 C4 01" );	// test	ah,0x01
					emitInstruction( "74 06" );		// je +6
					emitInstruction( "FF 25" );		// jmp	[0x12345678]
					v = constant4();
					jused[v] = 1;
					emit4( (int)instructionPointers + v*4 );
					break;			
				case OP(LEF):
					emitCommand(LAST_COMMAND_SUB_DI_8);		// sub edi, 8
					emitInstruction( "D9 47 04" );	// fld dword ptr [edi+4]
					emitInstruction( "D8 5F 08" );	// fcomp dword ptr [edi+8]
					emitInstruction( "DF E0" );		// fnstsw ax
					emitInstruction( "F6 C4 41" );	// test	ah,0x41
					emitInstruction( "74 06" );		// je +6
					emitInstruction( "FF 25" );		// jmp	[0x12345678]
					v = constant4();
					jused[v] = 1;
					emit4( (int)instructionPointers + v*4 );
					break;			
				case OP(GTF):
					emitCommand(LAST_COMMAND_SUB_DI_8);		// sub edi, 8
					emitInstruction( "D9 47 04" );	// fld dword ptr [edi+4]
					emitInstruction( "D8 5F 08" );	// fcomp dword ptr [edi+8]
					emitInstruction( "DF E0" );		// fnstsw ax
					emitInstruction( "F6 C4 41" );	// test	ah,0x41
					emitInstruction( "75 06" );		// jne +6
					emitInstruction( "FF 25" );		// jmp	[0x12345678]
					v = constant4();
					jused[v] = 1;
					emit4( (int)instructionPointers + v*4 );
					break;			
				case OP(GEF):
					emitCommand(LAST_COMMAND_SUB_DI_8);		// sub edi, 8
					emitInstruction( "D9 47 04" );	// fld dword ptr [edi+4]
					emitInstruction( "D8 5F 08" );	// fcomp dword ptr [edi+8]
					emitInstruction( "DF E0" );		// fnstsw ax
					emitInstruction( "F6 C4 01" );	// test	ah,0x01
					emitInstruction( "75 06" );		// jne +6
					emitInstruction( "FF 25" );		// jmp	[0x12345678]
					v = constant4();
					jused[v] = 1;
					emit4( (int)instructionPointers + v*4 );
					break;			
				case OP(NEGI):
					emitInstruction( "F7 1F" );		// neg dword ptr [edi]
					break;
				case OP(ADD):
					emitMovEAXEDI();			// mov eax, dword ptr [edi]
					emitInstruction( "01 47 FC" );	// add dword ptr [edi-4],eax
					emitCommand(LAST_COMMAND_SUB_DI_4);		// sub edi, 4
					break;
				case OP(SUB):
					emitMovEAXEDI();			// mov eax, dword ptr [edi]
					emitInstruction( "29 47 FC" );	// sub dword ptr [edi-4],eax
					emitCommand(LAST_COMMAND_SUB_DI_4);		// sub edi, 4
					break;
				case OP(DIVI):
					emitInstruction( "8B 47 FC" );	// mov eax,dword ptr [edi-4]
					emitInstruction( "99" );			// cdq
					emitInstruction( "F7 3F" );		// idiv dword ptr [edi]
					emitInstruction( "89 47 FC" );	// mov dword ptr [edi-4],eax
					emitCommand(LAST_COMMAND_SUB_DI_4);		// sub edi, 4
					break;
				case OP(DIVU):
					emitInstruction( "8B 47 FC" );	// mov eax,dword ptr [edi-4]
					emitInstruction( "33 D2" );		// xor edx, edx
					emitInstruction( "F7 37" );		// div dword ptr [edi]
					emitInstruction( "89 47 FC" );	// mov dword ptr [edi-4],eax
					emitCommand(LAST_COMMAND_SUB_DI_4);		// sub edi, 4
					break;
				case OP(MODI):
					emitInstruction( "8B 47 FC" );	// mov eax,dword ptr [edi-4]
					emitInstruction( "99" );			// cdq
					emitInstruction( "F7 3F" );		// idiv dword ptr [edi]
					emitInstruction( "89 57 FC" );	// mov dword ptr [edi-4],edx
					emitCommand(LAST_COMMAND_SUB_DI_4);		// sub edi, 4
					break;
				case OP(MODU):
					emitInstruction( "8B 47 FC" );	// mov eax,dword ptr [edi-4]
					emitInstruction( "33 D2" );		// xor edx, edx
					emitInstruction( "F7 37" );		// div dword ptr [edi]
					emitInstruction( "89 57 FC" );	// mov dword ptr [edi-4],edx
					emitCommand(LAST_COMMAND_SUB_DI_4);		// sub edi, 4
					break;
				case OP(MULI):
					emitInstruction( "8B 47 FC" );	// mov eax,dword ptr [edi-4]
					emitInstruction( "F7 2F" );		// imul dword ptr [edi]
					emitInstruction( "89 47 FC" );	// mov dword ptr [edi-4],eax
					emitCommand(LAST_COMMAND_SUB_DI_4);		// sub edi, 4
					break;
				case OP(MULU):
					emitInstruction( "8B 47 FC" );	// mov eax,dword ptr [edi-4]
					emitInstruction( "F7 27" );		// mul dword ptr [edi]
					emitInstruction( "89 47 FC" );	// mov dword ptr [edi-4],eax
					emitCommand(LAST_COMMAND_SUB_DI_4);		// sub edi, 4
					break;
				case OP(BAND):
					emitMovEAXEDI();			// mov eax, dword ptr [edi]
					emitInstruction( "21 47 FC" );	// and dword ptr [edi-4],eax
					emitCommand(LAST_COMMAND_SUB_DI_4);		// sub edi, 4
					break;
				case OP(BOR):
					emitMovEAXEDI();			// mov eax, dword ptr [edi]
					emitInstruction( "09 47 FC" );	// or dword ptr [edi-4],eax
					emitCommand(LAST_COMMAND_SUB_DI_4);		// sub edi, 4
					break;
				case OP(BXOR):
					emitMovEAXEDI();			// mov eax, dword ptr [edi]
					emitInstruction( "31 47 FC" );	// xor dword ptr [edi-4],eax
					emitCommand(LAST_COMMAND_SUB_DI_4);		// sub edi, 4
					break;
				case OP(BCOM):
					emitInstruction( "F7 17" );		// not dword ptr [edi]
					break;
				case OP(LSH):
					emitInstruction( "8B 0F" );		// mov ecx, dword ptr [edi]
					emitInstruction( "D3 67 FC" );	// shl dword ptr [edi-4], cl
					emitCommand(LAST_COMMAND_SUB_DI_4);		// sub edi, 4
					break;
				case OP(RSHI):
					emitInstruction( "8B 0F" );		// mov ecx, dword ptr [edi]
					emitInstruction( "D3 7F FC" );	// sar dword ptr [edi-4], cl
					emitCommand(LAST_COMMAND_SUB_DI_4);		// sub edi, 4
					break;
				case OP(RSHU):
					emitInstruction( "8B 0F" );		// mov ecx, dword ptr [edi]
					emitInstruction( "D3 6F FC" );	// shr dword ptr [edi-4], cl
					emitCommand(LAST_COMMAND_SUB_DI_4);		// sub edi, 4
					break;
				case OP(NEGF):
					emitInstruction( "D9 07" );		// fld dword ptr [edi]
					emitInstruction( "D9 E0" );		// fchs
					emitInstruction( "D9 1F" );		// fstp dword ptr [edi]
					break;
				case OP(ADDF):
					emitInstruction( "D9 47 FC" );	// fld dword ptr [edi-4]
					emitInstruction( "D8 07" );		// fadd dword ptr [edi]
					emitInstruction( "D9 5F FC" );	// fstp dword ptr [edi-4]
					emitCommand(LAST_COMMAND_SUB_DI_4);		// sub edi, 4
					break;
				case OP(SUBF):
					emitCommand(LAST_COMMAND_SUB_DI_4);		// sub edi, 4
					emitInstruction( "D9 07" );		// fld dword ptr [edi]
					emitInstruction( "D8 67 04" );	// fsub dword ptr [edi+4]
					emitInstruction( "D9 1F" );		// fstp dword ptr [edi]
					break;
				case OP(DIVF):
					emitCommand(LAST_COMMAND_SUB_DI_4);		// sub edi, 4
					emitInstruction( "D9 07" );		// fld dword ptr [edi]
					emitInstruction( "D8 77 04" );	// fdiv dword ptr [edi+4]
					emitInstruction( "D9 1F" );		// fstp dword ptr [edi]
					break;
				case OP(MULF):
					emitCommand(LAST_COMMAND_SUB_DI_4);		// sub edi, 4
					emitInstruction( "D9 07" );		// fld dword ptr [edi]
					emitInstruction( "D8 4f 04" );	// fmul dword ptr [edi+4]
					emitInstruction( "D9 1F" );		// fstp dword ptr [edi]
					break;
				case OP(CVIF):
					emitInstruction( "DB 07" );		// fild dword ptr [edi]
					emitInstruction( "D9 1F" );		// fstp dword ptr [edi]
					break;
				case OP(CVFI):
		#ifndef FTOL_PTR // WHENHELLISFROZENOVER  // bk001213 - was used in 1.17
					// not IEEE complient, but simple and fast
				  emitInstruction( "D9 07" );		// fld dword ptr [edi]
					emitInstruction( "DB 1F" );		// fistp dword ptr [edi]
		#else // FTOL_PTR
					// call the library conversion function
					emitInstruction( "D9 07" );		// fld dword ptr [edi]
					emitInstruction( "FF 15" );		// call ftolPtr
					emit4( (int)&ftolPtr );
					emitCommand(LAST_COMMAND_MOV_EDI_EAX);		// mov dword ptr [edi], eax
		#endif
					break;
				case OP(SEX8):
					emitInstruction( "0F BE 07" );	// movsx eax, byte ptr [edi]
					emitCommand(LAST_COMMAND_MOV_EDI_EAX);		// mov dword ptr [edi], eax
					break;
				case OP(SEX16):
					emitInstruction( "0F BF 07" );	// movsx eax, word ptr [edi]
					emitCommand(LAST_COMMAND_MOV_EDI_EAX);		// mov dword ptr [edi], eax
					break;

				case OP(BLOCK_COPY):
					// FIXME: range check
					emitInstruction( "56" );			// push esi
					emitInstruction( "57" );			// push edi
					emitInstruction( "8B 37" );		// mov esi,[edi] 
					emitInstruction( "8B 7F FC" );	// mov edi,[edi-4] 
					emitInstruction( "B9" );			// mov ecx,0x12345678
					emit4( constant4() >> 2 );
					emitInstruction( "B8" );			// mov eax, ramMask
					emit4( ramMask );
					emitInstruction( "BB" );			// mov ebx, ram
					emit4( (int)ram );
					emitInstruction( "23 F0" );		// and esi, eax
					emitInstruction( "03 F3" );		// add esi, ebx
					emitInstruction( "23 F8" );		// and edi, eax
					emitInstruction( "03 FB" );		// add edi, ebx
					emitInstruction( "F3 A5" );		// rep movsd
					emitInstruction( "5F" );			// pop edi
					emitInstruction( "5E" );			// pop esi
					emitCommand(LAST_COMMAND_SUB_DI_8);		// sub edi, 8
					break;

				case OP(JUMP):
					emitCommand(LAST_COMMAND_SUB_DI_4);		// sub edi, 4
					emitInstruction( "8B 47 04" );	// mov eax,dword ptr [edi+4]
					// FIXME: range check
					emitInstruction( "FF 24 85" );	// jmp dword ptr [instructionPointers + eax * 4]
					emit4( (int)instructionPointers );
					break;
				default:
					throw JITException("Compile: bad opcode");
				}
				pop0 = pop1;
				pop1 = op.Operation;
			}
		}

		// copy to an exact size buffer on the hunk
		//codeLength = compiledOfs;
		//codeBase = Hunk_Alloc( compiledOfs, h_low );
		
		//Com_Memcpy( codeBase, buf, compiledOfs );

		compiledRom = new char[compiledOfs];
		std::copy(buf, buf+compiledOfs, compiledRom);
		compiledRomSize = compiledOfs;
		compiledRomMask = compiledOfs;

		delete[] buf;
		delete[] jused;

		//printf( "VM file %s compiled to %i bytes of code\n", name, compiledOfs);

		// offset all the instruction pointers for the new location
		for ( i = 0 ; i < /*header->instructionCount*/ romSize ; i++ ) {
			instructionPointers[i] += (int)rom;
		}

#ifdef WIN32
		VirtualProtect(compiledRom, compiledRomSize, PAGE_EXECUTE, NULL);
#endif
	#if 0 // ndef _WIN32
		// Must make the newly generated code executable
		{
			int r;
			unsigned long addr;
			int psize = getpagesize();

			addr = ((int)codeBase & ~(psize-1)) - psize;

			r = mprotect((char*)addr, codeLength + (int)codeBase - addr + psize, 
				PROT_READ | PROT_WRITE | PROT_EXEC );

			if (r < 0)
				Com_Error( ERR_FATAL, "mprotect failed to change PROT_EXEC" );
		}
	#endif
		return true;

	}

	int VirtualMachine::constant4()
	{
		int v;

		v = rom[pc-1].Parameter.int4;// | (rom[pc-1].Parameter<<8) | (rom[pc-1].Parameter<<16) | (rom[pc-1].Parameter<<24);
		return v;
	}

	int VirtualMachine::constant1()
	{
		int		v;

		v = rom[pc-1].Parameter.uint1;
		return v;
	}

	void VirtualMachine::emit1(int v) 
	{
		buf[compiledOfs] = v;
		compiledOfs++;

		LastCommand = LAST_COMMAND_NONE;
	}

	void VirtualMachine::emit4(int v)
	{
		emit1(v & 255);
		emit1((v >> 8) & 255);
		emit1((v >> 16) & 255);
		emit1((v >> 24) & 255);
	}

	void VirtualMachine::emitInstruction(const char *string)
	{
		int		c1, c2;
		int		v;

		while (true)
		{
			c1 = string[0];
			c2 = string[1];

			v = (hex( c1 ) << 4) | hex(c2);
			emit1( v );

			if (!string[2])
			{
				break;
			}
			string += 3;
		}
	}

	void VirtualMachine::emitCommand(int command_)
	{
		ELastCommand command = (ELastCommand)command_;
		switch(command)
		{
			case LAST_COMMAND_MOV_EDI_EAX:
				emitInstruction( "89 07" );		// mov dword ptr [edi], eax
				break;
			case LAST_COMMAND_SUB_DI_4:
				emitInstruction( "83 EF 04" );	// sub edi, 4
				break;
			case LAST_COMMAND_SUB_DI_8:
				emitInstruction( "83 EF 08" );	// sub edi, 8
				break;
			default:
				break;
		}
		LastCommand = command;
	}

	void VirtualMachine::emitAddEDI4()
	{
		if (LastCommand == LAST_COMMAND_SUB_DI_4 && jused[instruction-1] == 0) 
		{		// sub di,4
			compiledOfs -= 3;
			instructionPointers[ instruction-1 ] = compiledOfs;
			return;
		}
		if (LastCommand == LAST_COMMAND_SUB_DI_8 && jused[instruction-1] == 0) 
		{		// sub di,8
			compiledOfs -= 3;
			instructionPointers[ instruction-1 ] = compiledOfs;
			emitInstruction( "83 EF 04" );	//	sub edi,4
			return;
		}
		emitInstruction( "83 C7 04" );	//	add edi,4
	}

	void VirtualMachine::emitMovEAXEDI()
	{
		if (LastCommand == LAST_COMMAND_MOV_EDI_EAX) 
		{	// mov [edi], eax
			compiledOfs -= 2;
			instructionPointers[ instruction-1 ] = compiledOfs;
			return;
		}
		if (pop1 == OP(DIVI) || pop1 == OP(DIVU) || pop1 == OP(MULI) || pop1 == OP(MULU) || pop1 == OP(STORE4) || pop1 == OP(STORE2) || pop1 == OP(STORE1) ) 
		{	
			return;
		}
		if (pop1 == OP(CONST) && buf[compiledOfs-6] == 0xC7 && buf[compiledOfs-5] == 0x07) 
		{	// mov edi, 0x123456
			compiledOfs -= 6;
			instructionPointers[ instruction-1 ] = compiledOfs;
			emitInstruction( "B8" );			// mov	eax, 0x12345678
			emit4( lastConst );
			return;
		}
		emitInstruction( "8B 07" );		// mov eax, dword ptr [edi]
	}

	bool VirtualMachine::emitMovEBXEDI(int andit)
	{
		if (LastCommand == LAST_COMMAND_MOV_EDI_EAX) 
		{	// mov [edi], eax
			compiledOfs -= 2;
			instructionPointers[ instruction-1 ] = compiledOfs;
			emitInstruction( "8B D8");		// mov bx, eax
			return false;
		}
		if (pop1 == OP(DIVI) || pop1 == OP(DIVU) || pop1 == OP(MULI) || pop1 == OP(MULU) || pop1 == OP(STORE4) || pop1 == OP(STORE2) || pop1 == OP(STORE1) ) 
		{	
			emitInstruction( "8B D8");		// mov bx, eax
			return false;
		}
		if (pop1 == OP(CONST) && buf[compiledOfs-6] == 0xC7 && buf[compiledOfs-5] == 0x07 ) 
		{		// mov edi, 0x123456
			compiledOfs -= 6;
			instructionPointers[ instruction-1 ] = compiledOfs;
			emitInstruction( "BB" );			// mov	ebx, 0x12345678
			if (andit) {
				emit4( lastConst & andit );
			} else {
				emit4( lastConst );
			}
			return true;
		}

		emitInstruction( "8B 1F" );		// mov ebx, dword ptr [edi]
		return false;
	}

	int VirtualMachine::hex(int c)
	{
		if (c >= 'a' && c <= 'f')
			return 10 + c - 'a';
		if (c >= 'A' && c <= 'F')
			return 10 + c - 'A';
		if (c >= '0' && c <= '9')
			return c - '0';

		throw JITException("hex: bad character");

		return 0;
	}

	#undef OP
}

#endif
