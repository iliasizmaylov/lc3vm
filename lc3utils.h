// FILE lc3utils.h

#ifndef _LC3UTILS_H_
#define _LC3UTILS_H_

#include "lc3def.h"

/**
 * Utility functions
 */

extern UINT signExnend(UINT input, UINT bitcount); // Sign extention to 16bits
extern void updFlags(_REGCODES reg); // Updating flags register according to a value of a certain register passed as "reg" arguement
extern void memwrite(UINT addr, UINT val); 
extern UINT memread(UINT addr);
/**
 * Instructions impementation
 */

// Instruction handler function type
// Input is a 16bit opcode
// returns nothing, works with global vars representing registers and memory
typedef void (*INSTRHANDLER)(UINT opcode);

/**
 *	Instruction handlers
 */

extern void instr_BR(UINT opcode); 
extern void instr_ADD(UINT opcode); 
extern void instr_LD(UINT opcode); 
extern void instr_ST(UINT opcode); 
extern void instr_JSR(UINT opcode); 
extern void instr_AND(UINT opcode); 
extern void instr_LDR(UINT opcode); 
extern void instr_STR(UINT opcode); 
extern void instr_RTI(UINT opcode); 
extern void instr_NOT(UINT opcode); 
extern void instr_LDI(UINT opcode); 
extern void instr_STI(UINT opcode); 
extern void instr_JMP(UINT opcode); 

// ---- reserved instruction here ----
// extern void instr_RES(UINT opcode);

extern void instr_LEA(UINT opcode); 
extern void instr_TRAP(UINT opcode); 

// Array of pointers to instruction
// Each instruction pointer has index within array according to it's opcode
INSTRHANDLER instrhandlers[OPCODE_COUNT];

extern void initInstructionHandlers();

#endif 

























