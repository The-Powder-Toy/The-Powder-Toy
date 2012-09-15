#include "VirtualMachine.h"

namespace vm
{
	#define OPDEF(n) &VirtualMachine::Op##n,
	OperationFunction VirtualMachine::operations[] = 
	{
		#include "Operations.inl"
	};
	#undef OPDEF

	#define OPDEF(n) int VirtualMachine::Op##n(word parameter)

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
		Marshal(parameter.uint1, Pop());
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
		R2.int4 = R0.int4 >> R1.int4;
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
