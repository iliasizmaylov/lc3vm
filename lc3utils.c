// FILE lc3utils.c

#include "lc3utils.h"

/**
 * Utiliy functions
 */

// Sign extention to 16bits
UINT signExtend(UINT input, int bitcount) {
    // if first bit is 1 then we are treating an input value as a negative value
    if ((input >> (bitcount - 1)) & 1) {
        input |= (MAX_MEM << bitcount);
    }

    return input;
}

// Updating flags register according to a value of a certain register passed as "reg" arguement
void updFlags(_REGCODES reg) {
    if (_REGS[reg] == 0) {
        _REGS[R_COND] = FLAG_ZRO;
    } else if (_REGS[reg] & BIT_NEG) {
        _REGS[R_COND] = FLAG_NEG;
    } else {
        _REGS[R_COND] = FLAG_POS;
    }
}   

// Write to memory
void memwrite(UINT addr, UINT val) {
    dbgprint("memwrite MAX_MEM: %u\n", MAX_MEM);
    dbgprint("memwrite addr: %u\n", (UINT) addr);
    if ((UINT) addr > (UINT) MAX_MEM) {
        fprintf(stderr, 
            "Virtualization error: trying to write to a wrong address: %u\n", 
            addr);
    } else {
        _MEM[(UINT) addr] = val; 
    }
}

// Read memory
UINT memread(UINT addr) {
    if (addr > MAX_MEM) { // if address is wrong then output an error
        fprintf(stderr,
            "Virtualization error: trying to read from a wrong address: %u\n",
            addr);
    } else if (addr == MMR_KBSR) { // else if we are reading keyboard register
        if (check_key()) {
            _MEM[MMR_KBSR] = (1 << (SYS_BITNESS - 1));
            _MEM[MMR_KBDR] = getchar();
        } else { // else plainly read a value from a memory address
            _MEM[MMR_KBSR] = 0x0;
        }
    }

    return _MEM[addr];
}

/**
 * Instruction handlers implementation
 */

// Add
void instr_ADD(UINT opcode) {
    UINT destReg = GET_DEST_REG(opcode);
    UINT sourceReg1 = GET_SRC_REG1(opcode);
    UINT immediate = GET_IMM_FLAG(opcode);

    dbgprint("ADD instruction destination register code: %x\n", destReg);
    dbgprint("ADD instruction desitnation register value %x\n", _REGS[destReg]);
    dbgprint("ADD instruction source register 1 code: %x\n", sourceReg1);
    dbgprint("ADD instruction source register 1 value %x\n", _REGS[sourceReg1]);
    dbgprint("ADD instruction immediate flag set: %x\n", immediate);

    if (immediate) {
        UINT imm5 = signExtend(GET_IMM_VAL(opcode), SIZE_IMM5);
        
        dbgprint("ADD instruction immediate value sign extended: %x\n", imm5);

        _REGS[destReg] = _REGS[sourceReg1] + imm5;
    } else {
        UINT sourceReg2 = GET_SRC_REG2(opcode);

        dbgprint("ADD instruction source register 2 code: %x\n", sourceReg2);
        dbgprint("ADD instruction source register 2 value: %x\n", _REGS[sourceReg2]);

        _REGS[destReg] = _REGS[sourceReg1] + _REGS[sourceReg2];
    }

    dbgprint("ADD instruction destination register value after: %x\n", _REGS[destReg]);

    updFlags(_REGS[destReg]);
}

// Load indirect
void instr_LDI(UINT opcode) {
    UINT destReg = GET_DEST_REG(opcode);
    UINT pcOffset = signExtend(GET_PC_OFFSET(opcode), SIZE_PCOFFSET9);

    dbgprint("LDI instruction destination register code: %x\n", destReg);
    dbgprint("LDI instruction destination register value: %x\n", _REGS[destReg]);
    dbgprint("LDI instruction pc offset: %x\n", pcOffset);
    dbgprint("LDI instruction reading value %x from memory at addr %x\n", memread(_REGS[R_PC] + pcOffset), _REGS[R_PC] + pcOffset);

    _REGS[destReg] = memread(memread(_REGS[R_PC] + pcOffset));

    dbgprint("LDI instruction reading value %x from memory at addr %x\n", _REGS[destReg], memread(_REGS[R_PC] + pcOffset));

    updFlags(destReg);
}

// Bitwise and
void instr_AND(UINT opcode) {
    UINT destReg = GET_DEST_REG(opcode);
    UINT sourceReg1 = GET_SRC_REG1(opcode);
    UINT immediate = GET_IMM_FLAG(opcode);

    if (immediate) {
        UINT imm5 = signExtend(GET_IMM_VAL(opcode), SIZE_IMM5);
        _REGS[destReg] = _REGS[sourceReg1] & imm5;
    } else {
        UINT sourceReg2 = GET_SRC_REG2(opcode);
        _REGS[destReg] = _REGS[sourceReg1] & _REGS[sourceReg2];
    }

    updFlags(destReg);
}

// Bitwise not
void instr_NOT(UINT opcode) {
    UINT destReg = GET_DEST_REG(opcode);
    UINT sourceReg1 = GET_SRC_REG1(opcode);

    _REGS[destReg] = ~_REGS[sourceReg1];

    updFlags(destReg);
}

// Branch
void instr_BR(UINT opcode) {
    UINT pcOffset = signExtend(GET_PC_OFFSET(opcode), SIZE_PCOFFSET9);
    UINT condFlag = GET_COND_FLAG(opcode);

    dbgprint("BR instruction pc offset: %x\n", pcOffset);
    dbgprint("BR instruction confition flag: %x\n", condFlag);
    dbgprint("BR instruction R_COND register value: %x\n", _REGS[R_COND]);

    if (condFlag & _REGS[R_COND]) {
        _REGS[R_PC] += pcOffset;
    }
}

// Jump (also handles RET)
void instr_JMP(UINT opcode) {
    dbgprint("jmp instruction input register code: %x\n", GET_JMP_BASER(opcode));
    dbgprint("jmp instruction input register value: %x\n", _REGS[GET_JMP_BASER(opcode)]);
    _REGS[R_PC] = _REGS[GET_JMP_BASER(opcode)];
}

// Jump register
void instr_JSR(UINT opcode) {
    UINT longFlag = GET_JSR_LONG_FLAG(opcode);
    _REGS[R_R7] = _REGS[R_PC];

    if (longFlag) {
        UINT longPCOffset = signExtend(GET_JSR_LONG_OFFSET(opcode), SIZE_PCOFFSET11);
        _REGS[R_PC] += longPCOffset;
    } else {
        UINT sourceReg1 = GET_SRC_REG1(opcode);
        _REGS[R_PC] = _REGS[sourceReg1];    
    }
}

// Load
void instr_LD(UINT opcode) {
    UINT destReg = GET_DEST_REG(opcode);
    UINT pcOffset = signExtend(GET_PC_OFFSET(opcode), SIZE_PCOFFSET9);
    
    dbgprint("LD instruction destination register code: %x\n", destReg);
    dbgprint("LD instruction destination register value: %x\n", _REGS[destReg]);
    dbgprint("LD instruction pc offset sign extended: %x\n", pcOffset);
    
    _REGS[destReg] = memread(_REGS[R_PC] + pcOffset);

    dbgprint("LD instruction memory addr to read from: %x\n", _REGS[R_PC] + pcOffset);
    dbgprint("LD instruction destination register value after: %x\n", _REGS[destReg]);

    updFlags(destReg);
}

// Load register
void instr_LDR(UINT opcode) {
    UINT destReg = GET_DEST_REG(opcode);
    UINT sourceReg1 = GET_SRC_REG1(opcode);
    UINT offset = signExtend(GET_OFFSET6(opcode), SIZE_OFFSET6);
    
    _REGS[destReg] = memread(_REGS[sourceReg1] + offset);
    updFlags(destReg);
}

// Load effective address
void instr_LEA(UINT opcode) {
    UINT destReg = GET_DEST_REG(opcode);
    UINT pcOffset = signExtend(GET_PC_OFFSET(opcode), SIZE_PCOFFSET9);

    _REGS[destReg] = _REGS[R_PC] + pcOffset;
    updFlags(destReg);
}

// Store
void instr_ST(UINT opcode) {
    UINT destReg = GET_DEST_REG(opcode);
    UINT pcOffset = signExtend(GET_PC_OFFSET(opcode), SIZE_PCOFFSET9);
    memwrite(_REGS[R_PC] + pcOffset, _REGS[destReg]);
}

// Store indirect
void instr_STI(UINT opcode) {
    UINT destReg = GET_DEST_REG(opcode);
    UINT pcOffset = signExtend(GET_PC_OFFSET(opcode), SIZE_PCOFFSET9);
    memwrite(memread(_REGS[R_PC] + pcOffset), _REGS[destReg]);
}

// Store register
void instr_STR(UINT opcode) {
    UINT destReg = GET_DEST_REG(opcode);
    UINT sourceReg1 = GET_SRC_REG1(opcode);
    UINT offset = signExtend(GET_OFFSET6(opcode), SIZE_OFFSET6);
    
    dbgprint("STR instruction destination register code: %x\n", destReg);
    dbgprint("STR instruction destination register value: %x\n", _REGS[destReg]);
    dbgprint("STR instruction source register 1 code: %x\n", sourceReg1);
    dbgprint("STR instruction source register 1 value: %x\n", _REGS[sourceReg1]);
    dbgprint("STR instruction offset sign extended: %x\n", offset);
    dbgprint("STR instruction offset not sign extended: %x\n", GET_OFFSET6(opcode));
    
    memwrite(_REGS[sourceReg1] + offset, _REGS[destReg]);

    dbgprint("STR instruction write to memory at addr: %x\n", _REGS[sourceReg1] + offset);
    dbgprint("STR instruction write to memory value: %x\n", _REGS[destReg]);
}

void instr_TRAP(UINT opcode) {
    switch(GET_TRAPVECT8(opcode)) {
        case TRAP_GETC:
            dbgprint("TRAP instruction called with TRAP_GETC\n");
            _REGS[R_R0] = (UINT) getchar();
            break;
        
        case TRAP_PUTS: {
            dbgprint("TRAP instruction called with TRAP_PUTS\n");
            UINT *memptr = _MEM + _REGS[R_R0];
            
            while (*memptr) {
                putc((char) *memptr, stdout);
                memptr += 1;
            }

            fflush(stdout);
            break;
        }

        case TRAP_OUT:
            dbgprint("TRAP instruction called with TRAP_OUT\n");
            putc((char)_REGS[R_R0], stdout);
            fflush(stdout);
            break;
        
        case TRAP_IN:
            dbgprint("TRAP instruction called with TRAP_IN\n");
            printf("User input: ");
            char userInput = getchar();
            putc(userInput, stdout);
            _REGS[R_R0] = (UINT) userInput;
            break;
        
        case TRAP_PUTSP: {
            dbgprint("TRAP instruction called with TRAP_PUTSP\n");
            UINT *mptr = _MEM + _REGS[R_R0];

            while (*mptr) {
                char byte1 = (*mptr) & 0xFF;
                putc(byte1, stdout);

                char byte2 = (*mptr) >> 8;
                if (byte2) {
                    putc(byte2, stdout);
                }

                mptr += 1;
            }

            fflush(stdout);
            break;
        }

        case TRAP_HALT:
            dbgprint("TRAP instruction called with TRAP_HALT\n");
            puts("HALT");
            fflush(stdout);
            _STATUS = STATUS_HALTED;
            break;

        default: 
            fprintf(stderr,
                "Unknown trap code %x, aborting...\n",
                GET_TRAPVECT8(opcode));
            abort();
            break;
    }
}

/**
 *  Saving instruction handler functions into an array of 
 *  pointers to instruction handler function
 */
void initInstructionHandlers() {
    instrhandlers[OP_BR]    = instr_BR;
    instrhandlers[OP_ADD]   = instr_ADD;
    instrhandlers[OP_LD]    = instr_LD;
    instrhandlers[OP_ST]    = instr_ST;
    instrhandlers[OP_JSR]   = instr_JSR;
    instrhandlers[OP_AND]   = instr_AND;
    instrhandlers[OP_LDR]   = instr_LDR;
    instrhandlers[OP_STR]   = instr_STR;

    // RTI instruction is disabled
    // instrhandlers[OP_RTI]    = instr_RTI;
    instrhandlers[OP_RTI]   = NULL;

    instrhandlers[OP_NOT]   = instr_NOT;
    instrhandlers[OP_LDI]   = instr_LDI;
    instrhandlers[OP_STI]   = instr_STI;
    instrhandlers[OP_JMP]   = instr_JMP;

    // ---- reserved instruction here ----
    // instrhandlers[OP_RES] = instr_RES;
    // actually initialized with NULL
    instrhandlers[OP_RES]   = NULL;

    instrhandlers[OP_LEA]   = instr_LEA;
    instrhandlers[OP_TRAP]  = instr_TRAP;
}


























