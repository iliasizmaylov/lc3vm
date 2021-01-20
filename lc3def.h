// FILE lc3def.h

#ifndef _LC3DEF_H_
#define _LC3DEF_H_

// Basic libraries
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <limits.h>
#include <stdarg.h>

// Some UNIX-specific stuff #1
// TODO: check out what it means
#include <unistd.h>
#include <fcntl.h>

// Some UNIX-specific stuff #2
// TODO: check out what it means
#include <sys/time.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/mman.h>

#define DEBUG_TRACE_ON

typedef uint8_t UINT8;
typedef uint16_t UINT;

#define MAX_MEM     UINT16_MAX
#define SYS_BITNESS sizeof(UINT) * CHAR_BIT

#define STATUS_RUNNING  1
#define STATUS_HALTED   0

UINT8 _STATUS; // System status

// Whole 16-bit memory storage (64KB of memory)
UINT _MEM[MAX_MEM];

typedef enum {
    R_R0 = 0,   // gpr 1 [0]
    R_R1,       // gpr 2 [1]
    R_R2,       // gpr 3 [2]
    R_R3,       // gpr 4 [3]
    R_R4,       // gpr 5 [4]
    R_R5,       // gpr 6 [5]
    R_R6,       // gpr 7 [6]
    R_R7,       // gpr 8 [7]
    R_PC,       // program counter [8]
    R_COND,     // condition flag [9]
    REGCOUNT    // total number of registers = 10
} _REGCODES;

// Memory mapped registers
#define MMR_KBSR 0xFE00 // Keyboard status register
#define MMR_KBDR 0xFE02 // keyboard data

// R_COND conditional register flags
#define FLAG_POS 1u << 0 // Positive 
#define FLAG_ZRO 1u << 1 // Zero
#define FLAG_NEG 1u << 2 // Negative

#define BIT_NEG 1u >> 15 // if most significant bit is set then the value is considered negative

// R_PC program counter register starting position
#define R_PC_START_ADDR 0x3000

// Register array
UINT _REGS[REGCOUNT];

// Order in accordance to LC-3 opcodes (BR is 0000, ADD is 0001, LD is 0010 and so on)
typedef enum {
    OP_BR = 0,      // branch
    OP_ADD,         // add
    OP_LD,          // load
    OP_ST,          // store
    OP_JSR,         // jump to subroutine
    OP_AND,         // bitwise and
    OP_LDR,         // load register
    OP_STR,         // store base + offset
    OP_RTI,         // return from interrupt (UNUSED)
    OP_NOT,         // bitwise not
    OP_LDI,         // load indirect
    OP_STI,         // store indirect
    OP_JMP,         // jump
    OP_RES,         // reserved
    OP_LEA,         // load effective address
    OP_TRAP,        // syscall
    OPCODE_COUNT    // total count of operations
} _OPCODES;

// Size for immediate value ADD and AND instructions which is 5 bits
#define IMMEDIATE_VAL_SIZE 5

// FIXME : these macroses might end up being unused
#define SIZE_DR             3
#define SIZE_SR1            3
#define SIZE_SR2            3
#define SIZE_IMM5           5
#define SIZE_PCOFFSET9      9
#define SIZE_BASER          3
#define SIZE_PCOFFSET11     11
#define SIZE_OFFSET6        6
#define SIZE_TRAPVECT8      8

// Macros for fetching operands from instructions
#define GET_DEST_REG(OP)        ((OP >> 9) & 0x7)   // get destination register code
#define GET_COND_FLAG(OP)       ((OP >> 9) & 0x7)   // get condition flag (same as getting destination register)
#define GET_SRC_REG1(OP)        ((OP >> 6) & 0x7)   // get source register 1 code
#define GET_JMP_BASER(OP)       ((OP >> 6) & 0x7)   // get base register for jump (same as getting src1 register)
#define GET_SRC_REG2(OP)        (OP & 0x7)          // get source register 2 code
#define GET_IMM_FLAG(OP)        ((OP >> 5) & 0x1)   // get immediate value flag
#define GET_IMM_VAL(OP)         (OP & 0x1F)         // get immediate value without sign extention
#define GET_PC_OFFSET(OP)       (OP & 0x1FF)        // get pcoffset withou sign extention
#define GET_JSR_LONG_FLAG(OP)   ((OP >> 11) & 0x1)  // get long flag for JSR
#define GET_JSR_LONG_OFFSET(OP) (OP & 0x7FF)        // get long pc offset for JSR without sign extention
#define GET_OFFSET6(OP)         (OP & 0x3F)         // get offset6 for LDR and STR without sign extention
#define GET_TRAPVECT8(OP)       (OP & 0xFF)         // get trap code

// Trap codes
#define TRAP_GETC   0x20    // getchar from keyboard
#define TRAP_OUT    0x21    // output a char
#define TRAP_PUTS   0x22    // output a word string
#define TRAP_IN     0x23    // getchar from keybord and echo to terminal
#define TRAP_PUTSP  0x24    // output a byte string
#define TRAP_HALT   0x25    // halt the program

/*
 * This section contains some tedious details that are needed to access the keyboard and behave nicely.
 */

extern void dbgprint(const char* format, ...);

struct termios original_tio;

extern UINT check_key();
extern void disable_input_buffering();
extern void restore_input_buffering();
extern void handle_interrupt(int signal);

/*
 * Reading lc-3 assembly image files
 */

extern UINT readImageFile(const char* imagePath); 

#endif













































