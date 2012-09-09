#include <string>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include "VirtualMachine.h"

namespace vm
{

	VirtualMachine::VirtualMachine(int hunkMbytes):
		bigEndian(false),
		hunk(NULL),
		hunkSize(1048576),
		hunkFree(0),
		rom(NULL),
		romSize(0),
		ram(NULL),
		ramSize(0),
		dataStack(0),
		returnStack(0),
		DP(0),        /* Datastack pointer. */
		RP(0),        /* Return stack pointer. */
		PC(0),
		cm(0),
		cycles(0)
	{
		hunk = new char[hunkSize];
		std::fill(hunk, hunk+hunkSize, 0);
	}

	VirtualMachine::~VirtualMachine()
	{
		delete[] hunk;
	}

	#define crumb printf

	int VirtualMachine::opcodeParameterSize(int opcode)
	{
		#define OP(n) OP##n
		switch (opcode)
		{
		case OP(ENTER):
		case OP(LEAVE):
		case OP(LOCAL):
		case OP(EQ):
		case OP(NE):
		case OP(LTI):
		case OP(LEI):
		case OP(GTI):
		case OP(GEI):
		case OP(LTU):
		case OP(LEU):
		case OP(GTU):
		case OP(GEU):
		case OP(EQF):
		case OP(NEF):
		case OP(LTF):
		case OP(LEF):
		case OP(GTF):
		case OP(GEF):
		case OP(CONST):
		case OP(BLOCK_COPY):
			return sizeof(uint4_t);
			break;
		case OP(ARG):
			return sizeof(uint1_t);
			break;
		}
		return 0;
		#undef OP
	}

	/* Read one octet from file. */
	int VirtualMachine::readByte(FILE *qvmfile)
	{
		int o;
		o = fgetc(qvmfile);
		if (o < 0) o = 0;  /* EOF (hack) */
			return o;
	}

	/* Read little-endian 32-bit integer from file. */
	int VirtualMachine::readInt(FILE *qvmfile)
	{
		int a, b, c, d, n;

		a = readByte(qvmfile);
		b = readByte(qvmfile);
		c = readByte(qvmfile);
		d = readByte(qvmfile);
		n = (a) | (b << 8) | (c << 16) | (d << 24);
		return n;
	}

	int VirtualMachine::LoadProgram(char * filename)
	{
		FILE * qvmfile = fopen(filename, "rb");
		qvm_header_t qvminfo;
		int i, n;
		uint1_t x[4];
		word w;

	crumb("Loading file...\n");
		qvminfo.magic = readInt(qvmfile); /* magic. */
		if (qvminfo.magic != QVM_MAGIC)
			{
				crumb("Invalid magic");
				//q3vm_error("Does not appear to be a QVM file.");
				/* XXX: option to force continue. */
				return 0;
			}
	crumb("Magic OK\n");
		/* variable-length instructions mean instruction count != code length */
		qvminfo.inscount = readInt(qvmfile);
		qvminfo.codeoff = readInt(qvmfile);
		qvminfo.codelen = readInt(qvmfile);
		qvminfo.dataoff = readInt(qvmfile);
		qvminfo.datalen = readInt(qvmfile);
		qvminfo.litlen = readInt(qvmfile);
		qvminfo.bsslen = readInt(qvmfile);

	/* Code segment should follow... */
	/* XXX: use fseek with SEEK_CUR? */
	crumb("Searching for .code @ %d from %d\n", qvminfo.codeoff, ftell(qvmfile));
	//  rom = (q3vm_rom_t*)(hunk);  /* ROM-in-hunk */
		rom = (Instruction*)calloc(qvminfo.inscount, sizeof(rom[0]));
		while (ftell(qvmfile) < qvminfo.codeoff)
				readByte(qvmfile);
		while (romSize < qvminfo.inscount)
			{
				n = readByte(qvmfile);
				w.int4 = 0;
				if ((i = opcodeParameterSize(n)))
					{
						x[0] = x[1] = x[2] = x[3] = 0;
						fread(&x, 1, i, qvmfile);
						w.uint4 = (x[0]) | (x[1] << 8) | (x[2] << 16) | (x[3] << 24);
					}
				rom[romSize].Operation = n;
				rom[romSize].Parameter = w;
				romSize++;
			}
	crumb("After loading code: at %d, should be %d\n", ftell(qvmfile), qvminfo.codeoff + qvminfo.codelen);

	/* Then data segment. */
	//  ram = hunk + ((romlen + 3) & ~3);  /* RAM-in-hunk */
		ram = hunk;
	crumb("Searching for .data @ %d from %d\n", qvminfo.dataoff, ftell(qvmfile));
		while (ftell(qvmfile) < qvminfo.dataoff)
				readByte(qvmfile);
		for (n = 0; n < (qvminfo.datalen / sizeof(uint1_t)); n++)
			{
				i = fread(&x, 1, sizeof(x), qvmfile);
				w.uint4 = (x[0]) | (x[1] << 8) | (x[2] << 16) | (x[3] << 24);
				*((word*)(ram + ramSize)) = w;
				ramSize += sizeof(word);
			}
	/* lit segment follows data segment. */
	/* Assembler should have already padded properly. */
	crumb("Loading .lit\n");
		for (n = 0; n < (qvminfo.litlen / sizeof(uint1_t)); n++)
			{
				i = fread(&x, 1, sizeof(x), qvmfile);
				memcpy(&(w.uint1), &x, sizeof(x));  /* no byte-swapping. */
				*((word*)(ram + ramSize)) = w;
				ramSize += sizeof(word);
			}
	/* bss segment. */
	crumb("Allocating .bss %d (%X) bytes\n", qvminfo.bsslen, qvminfo.bsslen);
		/* huge empty chunk. */
		ramSize += qvminfo.bsslen;

		hunkFree = hunkSize - ((ramSize * sizeof(uint1_t)) + 4);

	crumb("VM hunk has %d of %d bytes free (RAM = %d B).\n", hunkFree, hunkSize, ramSize);
		if (ramSize > hunkSize)
			{
				throw OutOfMemoryException();
				return 0;
			}

	/* set up stack. */
		{
			int stacksize = 0x10000;
			returnStack = ramSize;
			dataStack = ramSize - (stacksize / 2);
			RP = returnStack;
			DP = dataStack;
		}

	/* set up PC for return-to-termination. */
		PC = romSize + 1;

		return 1;
	}

	int VirtualMachine::Call(int address)
	{
		word w;
		int i, argCount = 13;

		/* Set up call. */
		opPUSH(w);
		crumb("Starting with PC=%d, DP=%d, RP=%d to %d\n", PC, DP, RP, address);
		w.int4 = (argCount + 2) * sizeof(word);
		opENTER(w);
		i = 8;
		/**w.int4 = arg0; Marshal(i, w); i += 4;
		w.int4 = arg1; Marshal(i, w); i += 4;
		w.int4 = arg2; Marshal(i, w); i += 4;
		w.int4 = arg3; Marshal(i, w); i += 4;
		w.int4 = arg4; Marshal(i, w); i += 4;
		w.int4 = arg5; Marshal(i, w); i += 4;
		w.int4 = arg6; Marshal(i, w); i += 4;
		w.int4 = arg7; Marshal(i, w); i += 4;
		w.int4 = arg8; Marshal(i, w); i += 4;
		w.int4 = arg9; Marshal(i, w); i += 4;
		w.int4 = arg10; Marshal(i, w); i += 4;
		w.int4 = arg11; Marshal(i, w); i += 4;
		w.int4 = arg12; Marshal(i, w); i += 4;*/
		w.int4 = address;
		Push(w);
		opCALL(w);
		printf("Upon running PC=%d, DP=%d, RP=%d\n", PC, DP, RP);
		Run();
		printf("At finish PC=%d, DP=%d, RP=%d\n", PC, DP, RP);
		w.int4 = (argCount + 2) * sizeof(word);
		opLEAVE(w);
		return 0;
	}

	int VirtualMachine::Run()
	{
		bool running = true;
		int operation;
		word parameter;
		while(running)
		{
			cycles++;
			if(PC > romSize)
			{
				running = false;
				continue;
			}
			if (PC < 0)
			{
				syscall(PC);
				continue;
			}
			operation = rom[PC].Operation;
			parameter = rom[PC].Parameter;
			PC++;
			(this->*operations[operation])(parameter);
		}
		return 1;
	}



	int VirtualMachine::syscall(int trap)
	{
		int retval;
		word w;

		retval = 0;
		switch (trap)
		{
		#define TRAPDEF(n, f) case n: retval = trap##f(); break;
			#include "Traps.inl"
		#undef TRAPDEF
		}

		w = Pop();
		PC = w.int4;
		w.int4 = retval;
		Push(w);
		return 1;
	}

	#define ARG(n) (Get<int4_t>(RP + ((2 + n) * sizeof(word))))

	#define TRAPDEF(f) int VirtualMachine::trap##f()

	TRAPDEF(Print)
	{
		char *text;

		//crumb("SYSCALL Print [%d]\n", ARG(0)); 
		text = (char*)(ram) + ARG(0);
		//crumb("PRINTING [%s]\n", text);
		printf("%s", text);
		return 0;
	}


	TRAPDEF(Error)
	{
		char *msg;

		msg = (char*)(ram) + ARG(0);
		printf("%s", msg);
		PC = romSize + 1;
		return 0;
	}

	#define OPDEF(n) &VirtualMachine::op##n,
	OperationFunction VirtualMachine::operations[] = 
	{
		#include "OpCodes.inl"
	};
	#undef OPDEF

	#define OPDEF(n) int VirtualMachine::op##n(word parameter)

	#define R0 (r[0])
	#define R1 (r[1])
	#define R2 (r[2])


	OPDEF(UNDEF)
	{
		/* Die horribly. */
		throw RuntimeException();
		return -1;
	}

	OPDEF(IGNORE)
	{
		/* NOP */
		throw RuntimeException();
		return 0;
	}

	OPDEF(BREAK)
	{
		/* Usage never spotted. */
		/* Die horribly? */
		throw RuntimeException();
		return -1;
	}

	/*
	Stack on entering...

	no locals: ENTER 8
	1 words locals: ENTER 16
	2 words locals: ENTER 20
	3 words locals: ENTER 24
	 etc.

	address of argument:
	 ADDRFP4 v  =>  OP_LOCAL (16 + currentLocals + currentArgs + v)
	address of local:
	 ADDRLP4 v  =>  OP_LOCAL (8 + currentArgs + v)

				RP	[        ]   ??? (oldPC?)
					[        ]   ???
					[        ]  \
						...        > locals (args marshalling)
					[        ]  /
					[        ]  \
						...        > locals
					[        ]  /     (ADDRLP4 v  =>  OP_LOCAL (8 + currentArgs + v))
	(oldRP?)		[        ]   ???
					[        ]   ???
					[        ]   (my args?)
						...
					[        ]
	*/

	OPDEF(ENTER)  /* ??? */
	{
		while (parameter.int4 > (2 * sizeof(word)))
		{
			RPush<int4_t>(0);  /* init zero */
			parameter.int4 -= sizeof(word);
		}
		RPush(Pop()); //Program Counter
		RPush<int4_t>(0);  //Unknown
		return 0;
	}

	OPDEF(LEAVE)  /* ??? */
	{
		RPop();  //Unknown
		parameter.int4 -= sizeof(word);
		PC = RPop<int4_t>();  //Program counter
		parameter.int4 -= sizeof(word);
		while (parameter.int4 > 0)
		{
			RPop();
			parameter.int4 -= sizeof(word);
		}
		return 0;
	}

	OPDEF(CALL)  /* Call subroutine. */
	{
		R0 = Pop();
		Push<int4_t>(PC);
		PC = R0.int4;
		return 0;
	}

	OPDEF(PUSH)  /* [DP] <- 0; DP++ */
	{
		Push(0);
		return 0;
	}

	OPDEF(POP)  /* DP-- */
	{
		Pop();
		return 0;
	}

	OPDEF(CONST)  /* [DP] <- parm; DP++ */
	{
		Push(parameter);
		return 0;
	}

	OPDEF(LOCAL)  /* [DP] <- [RP-n] */
	{
		Push<int4_t>(RP + parameter.int4);
		return 0;
	}

	OPDEF(JUMP)   /* PC <- [DP] */
	{
		PC = Pop<int4_t>();
		return 0;
	}

	#define CMP(type, op) \
	{ \
		R0 = Pop(); \
		cm = (Pop<type##_t>() op R0.type); \
		if (cm) \
			PC = parameter.uint4; \
		return 0; \
	}

	OPDEF(EQ)     /* if [DP] == [DP-1] then PC <- parm; DP <- DP-2 */
		CMP(int4, ==)

	OPDEF(NE)   /* if [DP] == [DP-1] then PC <- parm; DP <- DP-2 */
		CMP(int4, !=)

	OPDEF(LTI)  /* if [DP] < [DP-1] then PC <- parm; DP <- DP-2 */
		CMP(int4, <)

	OPDEF(LEI)  /* if [DP] <= [DP-1] then PC <- parm; DP <- DP-2 */
		CMP(int4, <=)

	OPDEF(GTI)  /* if [DP] > [DP-1] then PC <- parm; DP <- DP-2 */
		CMP(int4, >)

	OPDEF(GEI)  /* if [DP] >= [DP-1] then PC <- parm; DP <- DP-2 */
		CMP(int4, >=)

	OPDEF(LTU)  /* if [DP] < [DP-1] then PC <- parm; DP <- DP-2 */
		CMP(uint4, <)

	OPDEF(LEU)  /* if [DP] <= [DP-1] then PC <- parm; DP <- DP-2 */
		CMP(uint4, <=)

	OPDEF(GTU)  /* if [DP] > [DP-1] then PC <- parm; DP <- DP-2 */
		CMP(uint4, >)

	OPDEF(GEU)  /* if [DP] >= [DP-1] then PC <- parm; DP <- DP-2 */
		CMP(uint4, >=)

	OPDEF(EQF)  /* if [DP] == [DP-1] then PC <- parm; DP <- DP-2 */
		CMP(float4, ==)

	OPDEF(NEF)  /* if [DP] != [DP-1] then PC <- parm; DP <- DP-2 */
		CMP(float4, !=)

	OPDEF(LTF)  /* if [DP] < [DP-1] then PC <- parm; DP <- DP-2 */
		CMP(float4, <)

	OPDEF(LEF)  /* if [DP] <= [DP-1] then PC <- parm; DP <- DP-2 */
		CMP(float4, <=)

	OPDEF(GTF)  /* if [DP] > [DP-1] then PC <- parm; DP <- DP-2 */
		CMP(float4, >)

	OPDEF(GEF)  /* if [DP] >= [DP-1] then PC <- parm; DP <- DP-2 */
		CMP(float4, >=)


	OPDEF(LOAD1)  /* [DP] <- [[DP]] */
	{
		Push<uint1_t>(Get<uint1_t>(Pop<uint4_t>()));
		return 0;
	}

	OPDEF(LOAD2)  /* [DP] <- [[DP]] */
	{
		Push<uint2_t>(Get<uint2_t>(Pop<uint4_t>()));
		return 0;
	}

	OPDEF(LOAD4)  /* [DP] <- [[DP]] */
	{
		Push<uint4_t>(Get<uint4_t>(Pop<uint4_t>()));
		return 0;
	}

	OPDEF(STORE1) /* [DP-1] <- [DP]; DP <- DP-2 */
	{
		Set<uint1_t>(Pop<uint4_t>(), Pop<uint1_t>());
		return 0;
	}

	OPDEF(STORE2) /* [DP-1] <- [DP]; DP <- DP-2 */
	{
		Set<uint2_t>(Pop<uint4_t>(), Pop<uint2_t>());
		return 0;
	}

	OPDEF(STORE4) /* [DP-1] <- [DP]; DP <- DP-2 */
	{
		Set<uint4_t>(Pop<uint4_t>(), Pop<uint4_t>());
		return 0;
	}

	OPDEF(ARG)    /* Marshal TOS to to-call argument list */
	{
		Marshal(parameter.int4, Pop());
		return 0;
	}

	OPDEF(BLOCK_COPY)  /* XXX */
	{
		R1 = Pop();
		R0 = Pop();
		if(R0.int4 >= 0 && R0.int4 + parameter.int4 < ramSize && R1.int4 >= 0 && R1.int4 + parameter.int4 < ramSize)
			memcpy(ram + R0.int4, ram + R1.int4, parameter.int4);
		else
			throw AccessViolationException();
		return -1;
	}

	OPDEF(SEX8)   /* Sign-extend 8-bit */
	{
		R0 = Pop();
		if(R0.uint4 & 0x80)
			R0.uint4 |= 0xFFFFFF80;
		Push(R0);
		return 0;
	}

	OPDEF(SEX16)  /* Sign-extend 16-bit */
	{
		R0 = Pop();
		if(R0.uint4 & 0x8000)
			R0.uint4 |= 0xFFFF8000;
		Push(R0);
		return 0;
	}

	#define UNOP(type, op) \
	{ \
		Push<type##_t>(op Pop<type##_t>()); \
		return 0; \
	} 

	#define BINOP(type, op) \
	{ \
		R0 = Pop(); \
		Push<type##_t>(Pop<type##_t>() op R0.type); \
		return 0; \
	}

	OPDEF(NEGI)  /* [DP] <- -[DP] */
		UNOP(int4, -)

	OPDEF(ADD)   /* [DP-1] <- [DP-1] + [DP]; DP <- DP-1 */
		BINOP(int4, +)

	OPDEF(SUB)   /* [DP-1] <- [DP-1] - [DP]; DP <- DP-1 */
		BINOP(int4, -)

	OPDEF(DIVI)   /* [DP-1] <- [DP-1] / [DP]; DP <- DP-1 */
		BINOP(int4, /)

	OPDEF(DIVU)   /* [DP-1] <- [DP-1] / [DP]; DP <- DP-1 */
		BINOP(uint4, /)

	OPDEF(MODI)   /* [DP-1] <- [DP-1] % [DP]; DP <- DP-1 */
		BINOP(int4, %)

	OPDEF(MODU)   /* [DP-1] <- [DP-1] % [DP]; DP <- DP-1 */
		BINOP(uint4, %)

	OPDEF(MULI)   /* [DP-1] <- [DP-1] * [DP]; DP <- DP-1 */
		BINOP(int4, *)

	OPDEF(MULU)   /* [DP-1] <- [DP-1] * [DP]; DP <- OP-1 */
		BINOP(uint4, *)

	OPDEF(BAND)   /* [DP-1] <- [DP-1] & [DP]; DP <- DP-1 */
		BINOP(uint4, &)

	OPDEF(BOR)   /* [DP-1] <- [DP-1] | [DP]; DP <- DP-1 */
		BINOP(uint4, |)

	OPDEF(BXOR)   /* [DP-1] <- [DP-1] ^ [DP]; DP <- DP-1 */
		BINOP(uint4, ^)

	OPDEF(BCOM)   /* [DP] <- ~[DP] */
		UNOP(uint4, ~)

	OPDEF(LSH)   /* [DP-1] <- [DP-1] << [DP]; DP <- DP-1 */
		BINOP(uint4, <<)

	OPDEF(RSHI)  /* [DP-1] <- [DP-1] >> [DP]; DP <- DP-1 */
	{
		R1.int4 = Pop<int4_t>();
		R0.int4 = Pop<int4_t>();
		#if 0
		while (R1.int4-- > 0)
			R0.int4 /= 2;
		R2 = R0;
		#else
		R2.int4 = R0.int4 >> R1.int4;
		#endif
		Push(R2);
		return 0;
	}

	OPDEF(RSHU)   /* [DP-1] <- [DP-1] >> [DP]; DP <- DP-1 */
		BINOP(uint4, >>)

	OPDEF(NEGF)   /* [DP] <- -[DP] */
		UNOP(float4, -)

	OPDEF(ADDF)   /* [DP-1] <- [DP-1] + [DP]; DP <- DP-1 */
		BINOP(float4, +)

	OPDEF(SUBF)   /* [DP-1] <- [DP-1] - [DP]; DP <- DP-1 */
		BINOP(float4, -)

	OPDEF(DIVF)   /* [DP-1] <- [DP-1] / [DP]; DP <- DP-1 */
		BINOP(float4, /)

	OPDEF(MULF)   /* [DP-1] <- [DP-1] / [DP]; DP <- DP-1 */
		BINOP(float4, *)

	OPDEF(CVIF)   /* [DP] <- [DP] */
	{
		Push<float4_t>(Pop<int4_t>());
		return 0;
	}

	OPDEF(CVFI)   /* [DP] <- [DP] */
	{
		Push<int4_t>(Pop<float4_t>());
		return 0;
	}
}