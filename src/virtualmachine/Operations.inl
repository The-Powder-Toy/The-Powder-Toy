OPDEF(UNDEF)
OPDEF(IGNORE)  /* no-op */
OPDEF(BREAK)   /* ??? */
OPDEF(ENTER)   /* Begin subroutine. */
OPDEF(LEAVE)   /* End subroutine. */
OPDEF(CALL)    /* Call subroutine. */
OPDEF(PUSH)    /* push to stack. */
OPDEF(POP)     /* discard top-of-stack. */
OPDEF(CONST)   /* load constant to stack. */
OPDEF(LOCAL)   /* get local variable. */
OPDEF(JUMP)    /* unconditional jump. */
OPDEF(EQ)      /* compare integers, jump if equal. */
OPDEF(NE)      /* compare integers, jump if not equal. */
OPDEF(LTI)     /* compare integers, jump if less-than. */
OPDEF(LEI)     /* compare integers, jump if less-than-or-equal. */
OPDEF(GTI)     /* compare integers, jump if greater-than. */
OPDEF(GEI)     /* compare integers, jump if greater-than-or-equal. */
OPDEF(LTU)     /* compare unsigned integers, jump if less-than */
OPDEF(LEU)     /* compare unsigned integers, jump if less-than-or-equal */
OPDEF(GTU)     /* compare unsigned integers, jump if greater-than */
OPDEF(GEU)     /* compare unsigned integers, jump if greater-than-or-equal */
OPDEF(EQF)     /* compare floats, jump if equal */
OPDEF(NEF)     /* compare floats, jump if not-equal */
OPDEF(LTF)     /* compare floats, jump if less-than */
OPDEF(LEF)     /* compare floats, jump if less-than-or-equal */
OPDEF(GTF)     /* compare floats, jump if greater-than */
OPDEF(GEF)     /* compare floats, jump if greater-than-or-equal */
OPDEF(LOAD1)   /* load 1-byte from memory */
OPDEF(LOAD2)   /* load 2-byte from memory */
OPDEF(LOAD4)   /* load 4-byte from memory */
OPDEF(STORE1)  /* store 1-byte to memory */
OPDEF(STORE2)  /* store 2-byte to memory */
OPDEF(STORE4)  /* store 4-byte to memory */
OPDEF(ARG)     /* marshal argument */
OPDEF(BLOCK_COPY) /* block copy... */
OPDEF(SEX8)    /* Pedophilia */
OPDEF(SEX16)   /* Sign-Extend 16-bit */
OPDEF(NEGI)    /* Negate integer. */
OPDEF(ADD)     /* Add integers (two's complement). */
OPDEF(SUB)     /* Subtract integers (two's complement). */
OPDEF(DIVI)    /* Divide signed integers. */
OPDEF(DIVU)    /* Divide unsigned integers. */
OPDEF(MODI)    /* Modulus (signed). */
OPDEF(MODU)    /* Modulus (unsigned). */
OPDEF(MULI)    /* Multiply signed integers. */
OPDEF(MULU)    /* Multiply unsigned integers. */
OPDEF(BAND)    /* Bitwise AND */
OPDEF(BOR)     /* Bitwise OR */
OPDEF(BXOR)    /* Bitwise eXclusive-OR */
OPDEF(BCOM)    /* Bitwise COMplement */
OPDEF(LSH)     /* Left-shift */
OPDEF(RSHI)    /* Right-shift (algebraic; preserve sign) */
OPDEF(RSHU)    /* Right-shift (bitwise; ignore sign) */
OPDEF(NEGF)    /* Negate float */
OPDEF(ADDF)    /* Add floats */
OPDEF(SUBF)    /* Subtract floats */
OPDEF(DIVF)    /* Divide floats */
OPDEF(MULF)    /* Multiply floats */
OPDEF(CVIF)    /* Convert to integer from float */
OPDEF(CVFI)    /* Convert to float from integer */