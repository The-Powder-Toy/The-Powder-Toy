#pragma once

#include "Exceptions.h"

namespace vm
{

	class VirtualMachine;

	typedef char ram_t;



	typedef unsigned	int		uint4_t;
	typedef signed		int		int4_t;

	typedef unsigned	short	uint2_t;
	typedef signed		short	int2_t;

	typedef unsigned	char	uint1_t;
	typedef signed		char	int1_t;

	typedef				float	float4_t;

	union word
	{
		uint4_t		uint4;
		int4_t		int4;
		uint2_t		uint2;
		int2_t		int2;
		uint1_t		uint1;
		int1_t		int1;
		float4_t		float4;
	};

	typedef int (VirtualMachine::*OperationFunction)(word parameter);

	struct Instruction
	{
		int Operation;
		word Parameter;
		//opfunc opfunc;
	};

	enum
	{
		QVM_MAGIC = 0x12721444,
	};

	struct qvm_header_t
	{
		int magic;
		/* not-entirely-RISC ISA, so instruction count != codelen */
		int inscount; /* instruction count. */
		int codeoff;  /* file offset of code segment. */
		int codelen;  /* length of code segment, in octets. */
		int dataoff;  /* file offset of data segment. */
		int datalen;  /* length of data segment, in octets. */
		int litlen;   /* length of lit segment (which is embedded in data segment). */
		int bsslen;   /* length of bss segment. */
	};

	class VirtualMachine
	{
		bool bigEndian;  /* host is big-endian (requires byte-swapping). */

		/* Memory spaces. */
		char * hunk;    /* hunk space (malloc'd). */
		int hunkSize;  /* total hunk size. */
		int hunkFree;  /* free pointer. */
		
		/* Read-Only Memory (code). */
		Instruction * rom;
		int romSize;

		/* Random-Access Memory (data). */
		ram_t *ram;
		int ramSize;

		int dataStack;
		int returnStack;

		word r[4];  	/* registers. */
		int DP;			/* Datastack pointer. */
		int RP;			/* Return stack pointer. */
		int PC;			/* Program Counter. */
		//  int AP;        /* Argument pointer.  (hrm...) */

		/* various flags. */
		int cm:1;
		
		/* Execution time */
		int cycles;

		#define TRAPDEF(n, f) int trap##f();
			#include "Syscalls.inl"
		#undef TRAPDEF

		static OperationFunction operations[];


		#define OPDEF(n) OP##n,
		enum {
			#include "Operations.inl"
		};
		#undef OPDEF

		int readByte(FILE *qvmfile);
		int readInt(FILE *qvmfile);
		int opcodeParameterSize(int opcode);
		int syscall(int programCounter);
public:
		#define OPDEF(n) int Op##n(word parameter);
		#include "Operations.inl"
		#undef OPDEF

		VirtualMachine(int hunkMbytes);
		virtual ~VirtualMachine();

		int LoadProgram(char * filename);
		int Run();
		int Call(int address);
		void End();
		void Marshal(int address, word element)
		{
			ram_t * ptr = ram+RP+address;
			if(ptr < ram || ptr > ram+ramSize - sizeof(word))
				throw AccessViolationException(RP+address);
			*((word*)ptr) = element;
		}

		template <typename T> T Get(int address)
		{
			ram_t * ptr = ram+address;
			if(ptr < ram || ptr > ram+ramSize - sizeof(word))
				throw AccessViolationException(address);
			return *((T*)ptr);
		}

		template <typename T> void Set(int address, T value)
		{
			ram_t * ptr = ram+address;
			if(ptr < ram || ptr > ram+ramSize - sizeof(word))
				throw AccessViolationException(address);
			*((T*)ptr) = value;
		}

		template <typename T> T Pop ()
		{
			ram_t * ptr = ram+DP;
			if(DP + sizeof(word) < hunkSize)
				DP += sizeof(word);
			else
				throw StackUnderflowException();
			return *((T*)ptr);
		};

		template <typename T> T RPop ()
		{
			ram_t * ptr = ram+RP;
			if(RP + sizeof(word) < hunkSize)
				RP += sizeof(word);
			else
				throw StackUnderflowException();
			return *((T*)ptr);
		};

		template <typename T> void Push(T value)
		{
			if(DP - sizeof(word) >= 0)
				DP -= sizeof(word);
			else
				throw StackOverflowException();
			ram_t * ptr = ram+DP;
			*((T*)ptr) = value;
		};

		template <typename T> void RPush(T value)
		{
			if(RP - sizeof(word) >= 0)
				RP -= sizeof(word);
			else
				throw StackOverflowException();
			ram_t * ptr = ram+RP;
			*((T*)ptr) = value;
		};

		word Get(int address)
		{
			ram_t * ptr = ram+address;
			if(ptr < ram || ptr > ram+ramSize - sizeof(word))
				throw AccessViolationException(address);
			return *((word*)ptr);
		}

		void Set(int address, word value)
		{
			ram_t * ptr = ram+address;
			if(ptr < ram || ptr > ram+ramSize - sizeof(word))
				throw AccessViolationException(address);
			*((word*)ptr) = value;
		}

		word Pop()
		{
			ram_t * ptr = ram+DP;
			if(DP + sizeof(word) < hunkSize)
				DP += sizeof(word);
			else
				throw StackUnderflowException();
			return *((word*)ptr);
		};

		void Push(word value)
		{
			if(DP - sizeof(word) >= 0)
				DP -= sizeof(word);
			else
				throw StackOverflowException();
			ram_t * ptr = ram+DP;
			*((word*)ptr) = value;
		};

		word RPop()
		{
			ram_t * ptr = ram+RP;
			if(RP + sizeof(word) < hunkSize)
				RP += sizeof(word);
			else
				throw StackUnderflowException();
			return *((word*)ptr);
		};

		void RPush(word value)
		{
			if(RP - sizeof(word) >= 0)
				RP -= sizeof(word);
			else
				throw StackOverflowException();
			ram_t * ptr = ram+RP;
			*((word*)ptr) = value;
		};
	};

}