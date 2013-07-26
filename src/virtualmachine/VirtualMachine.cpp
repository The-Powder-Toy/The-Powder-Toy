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
		cycles(0),
		sim(NULL),
		ren(NULL)
	{
		hunk = new char[hunkSize];
		std::fill(hunk, hunk+hunkSize, 0);
	}

	VirtualMachine::~VirtualMachine()
	{
		delete[] hunk;
	}

	#define DEBUGTRACE(args, ...) printf(args);

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
	int VirtualMachine::readByte(std::istream & input)
	{
		int o;
		o = input.get();
		if (o < 0) o = 0;  /* EOF (hack) */
			return o;
	}

	/* Read little-endian 32-bit integer from file. */
	int VirtualMachine::readInt(std::istream & input)
	{
		int a, b, c, d, n;

		a = readByte(input);
		b = readByte(input);
		c = readByte(input);
		d = readByte(input);
		n = (a) | (b << 8) | (c << 16) | (d << 24);
		return n;
	}

	int VirtualMachine::readProgram(std::istream & input)
	{
		qvm_header_t qvminfo;
		int i, n;
		uint1_t x[4];
		word w;

		DEBUGTRACE("Loading file...\n");
		qvminfo.magic = readInt(input); /* magic. */
		if (qvminfo.magic != QVM_MAGIC)
		{
			DEBUGTRACE("Invalid magic");
			throw InvalidProgramException();
			//q3vm_error("Does not appear to be a QVM file.");
			/* XXX: option to force continue. */
			return 0;
		}
		DEBUGTRACE("Magic OK\n");
		/* variable-length instructions mean instruction count != code length */
		qvminfo.inscount = readInt(input);
		qvminfo.codeoff = readInt(input);
		qvminfo.codelen = readInt(input);
		qvminfo.dataoff = readInt(input);
		qvminfo.datalen = readInt(input);
		qvminfo.litlen = readInt(input);
		qvminfo.bsslen = readInt(input);

		/* Code segment should follow... */
		/* XXX: use fseek with SEEK_CUR? */
		DEBUGTRACE("Searching for .code @ %d from %d\n", qvminfo.codeoff, input.tellg());

		//  rom = (q3vm_rom_t*)(hunk);  /* ROM-in-hunk */
		rom = (Instruction*)calloc(qvminfo.inscount, sizeof(rom[0]));
		while (input.tellg() < qvminfo.codeoff)
			readByte(input);
		while (romSize < qvminfo.inscount)
		{
			n = readByte(input);
			w.int4 = 0;
			if ((i = opcodeParameterSize(n)))
			{
				x[0] = x[1] = x[2] = x[3] = 0;
				input.readsome((char*)x, i);
				w.uint4 = (x[0]) | (x[1] << 8) | (x[2] << 16) | (x[3] << 24);
			}
			rom[romSize].Operation = n;
			rom[romSize].Parameter = w;
			romSize++;
		}
		DEBUGTRACE("After loading code: at %d, should be %d\n", input.tellg(), qvminfo.codeoff + qvminfo.codelen);

		/* Then data segment. */
		//  ram = hunk + ((romlen + 3) & ~3);  /* RAM-in-hunk */
		ram = hunk;
		DEBUGTRACE("Searching for .data @ %d from %d\n", qvminfo.dataoff, input.tellg());
		while (input.tellg() < qvminfo.dataoff)
			readByte(input);
		for (n = 0; n < (qvminfo.datalen / sizeof(uint1_t)); n++)
		{
			i = input.readsome((char*)x, sizeof(x));
			w.uint4 = (x[0]) | (x[1] << 8) | (x[2] << 16) | (x[3] << 24);
			*((word*)(ram + ramSize)) = w;
			ramSize += sizeof(word);
		}
	
		/* lit segment follows data segment. */
		/* Assembler should have already padded properly. */
		DEBUGTRACE("Loading .lit\n");
		for (n = 0; n < (qvminfo.litlen / sizeof(uint1_t)); n++)
		{
			i = input.readsome((char*)x, sizeof(x));
			memcpy(&(w.uint1), &x, sizeof(x));  /* no byte-swapping. */
			*((word*)(ram + ramSize)) = w;
			ramSize += sizeof(word);
		}
		/* bss segment. */
		DEBUGTRACE("Allocating .bss %d (%X) bytes\n", qvminfo.bsslen, qvminfo.bsslen);
		/* huge empty chunk. */
		ramSize += qvminfo.bsslen;

		hunkFree = hunkSize - ((ramSize * sizeof(uint1_t)) + 4);

		DEBUGTRACE("VM hunk has %d of %d bytes free (RAM = %d B).\n", hunkFree, hunkSize, ramSize);
		if (ramSize > hunkSize)
		{
			throw OutOfMemoryException();
			return 0;
		}

		/* set up stack. */
		{
			int stacksize = 0x10000;
			dataStack = ramSize - (stacksize / 2);
			returnStack = ramSize;
			//returnStack = dataStack+4;
			RP = returnStack;
			DP = dataStack;
		}

		/* set up PC for return-to-termination. */
		PC = romSize + 1;

		ramMask = ramSize;

		return 1;
	}

	int VirtualMachine::LoadProgram(std::vector<char> data)
	{
		/*class vectorwrapbuf : public std::basic_streambuf<char, std::char_traits<char> >
		{
		public:
    		vectorwrapbuf(std::vector<char> &vec) {
    		    setg(vec.data(), vec.data(), vec.data() + vec.size());
    		}
		};
		vectorwrapbuf databuf(data);
		std::istream is(&databuf);
		return readProgram(is);*/
		std::stringstream ss(std::string(data.begin(), data.end()));
		return readProgram((std::istream &)ss);
	}

	int VirtualMachine::LoadProgram(char * filename)
	{
		/*FILE * qvmfile = fopen(filename, "rb");
		qvm_header_t qvminfo;
		int i, n;
		uint1_t x[4];
		word w;

		DEBUGTRACE("Loading file...\n");
		qvminfo.magic = readInt(qvmfile);
		if (qvminfo.magic != QVM_MAGIC)
			{
				DEBUGTRACE("Invalid magic");
				return 0;
			}
		DEBUGTRACE("Magic OK\n");

		qvminfo.inscount = readInt(qvmfile);
		qvminfo.codeoff = readInt(qvmfile);
		qvminfo.codelen = readInt(qvmfile);
		qvminfo.dataoff = readInt(qvmfile);
		qvminfo.datalen = readInt(qvmfile);
		qvminfo.litlen = readInt(qvmfile);
		qvminfo.bsslen = readInt(qvmfile);


		DEBUGTRACE("Searching for .code @ %d from %d\n", qvminfo.codeoff, ftell(qvmfile));

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
		DEBUGTRACE("After loading code: at %d, should be %d\n", ftell(qvmfile), qvminfo.codeoff + qvminfo.codelen);


		ram = hunk;
		DEBUGTRACE("Searching for .data @ %d from %d\n", qvminfo.dataoff, ftell(qvmfile));
		while (ftell(qvmfile) < qvminfo.dataoff)
			readByte(qvmfile);
		for (n = 0; n < (qvminfo.datalen / sizeof(uint1_t)); n++)
		{
			i = fread(&x, 1, sizeof(x), qvmfile);
			w.uint4 = (x[0]) | (x[1] << 8) | (x[2] << 16) | (x[3] << 24);
			*((word*)(ram + ramSize)) = w;
			ramSize += sizeof(word);
		}
	

		DEBUGTRACE("Loading .lit\n");
		for (n = 0; n < (qvminfo.litlen / sizeof(uint1_t)); n++)
		{
			i = fread(&x, 1, sizeof(x), qvmfile);
			memcpy(&(w.uint1), &x, sizeof(x));
			*((word*)(ram + ramSize)) = w;
			ramSize += sizeof(word);
		}

		DEBUGTRACE("Allocating .bss %d (%X) bytes\n", qvminfo.bsslen, qvminfo.bsslen);
		ramSize += qvminfo.bsslen;

		hunkFree = hunkSize - ((ramSize * sizeof(uint1_t)) + 4);

		DEBUGTRACE("VM hunk has %d of %d bytes free (RAM = %d B).\n", hunkFree, hunkSize, ramSize);
		if (ramSize > hunkSize)
		{
			throw OutOfMemoryException();
			return 0;
		}


		{
			int stacksize = 0x10000;
			dataStack = ramSize - (stacksize / 2);
			//returnStack = ramSize;
			returnStack = dataStack+4;
			RP = returnStack;
			DP = dataStack;
		}


		PC = romSize + 1;

		ramMask = ramSize;

		return 1;*/
		return 0; //temporary, something has to be returned for now
	}

	void VirtualMachine::End()
	{
		PC = romSize+1;
	}

	int VirtualMachine::CallInterpreted(int address)
	{
		word w;
		int i, argCount = 0;

		/* Set up call. */
		OpPUSH(w);
		DEBUGTRACE("Starting with PC=%d, DP=%d, RP=%d to %d\n", PC, DP, RP, address);
		w.int4 = (argCount + 2) * sizeof(word);
		OpENTER(w);
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
		OpCALL(w);
		DEBUGTRACE("Upon running PC=%d, DP=%d, RP=%d\n", PC, DP, RP);
		Run();
		DEBUGTRACE("At finish PC=%d, DP=%d, RP=%d\n", PC, DP, RP);
		w.int4 = (argCount + 2) * sizeof(word);
		OpLEAVE(w);
		OpPOP(w);
		PC = romSize + 1;
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
		PC = Pop<int4_t>();

		switch (trap)
		{
		#define TRAPDEF(n, f) case n: trap##f(); break;
			#include "Syscalls.inl"
		#undef TRAPDEF
		}

		return 1;
	}
}
