// FILE lc3main.c

#include "lc3utils.h"

int main (int argc, char *argv[]) {
	// Loading input arguments which are LC-3 assemly image files (*.img)
	// checking if there are any input files specified
	if (argc < 2) {
		fprintf(stderr, 
			"usage: lc3vm [image file 1], ..., [image file n]\n");

		exit(2);
	}	

	// loading all specified image files into memory
	for (int i = 1; i < argc; i++) {
		if (!readImageFile(argv[i])) {
			// print error if a certain image file can not be processed and exit
			fprintf(stderr, 
				"Failed to load image file \"%s\"\n", 
				argv[i]);

			exit(1);
		}
	}

	// Signalling an interrupt and disabling input buffering
	// this way we're setting up a VM
	// TODO: look it up 
	signal(SIGINT, handle_interrupt);
	disable_input_buffering();
	initInstructionHandlers();

	// Program start address (by default in LC-3 it's 0x3000)
	_REGS[R_PC] = R_PC_START_ADDR;
	_STATUS = STATUS_RUNNING;
	
	while (_STATUS == STATUS_RUNNING) {
		UINT addrBackup = _REGS[R_PC];
		UINT nextInstruction = memread(_REGS[R_PC]++);
		UINT op = nextInstruction >> 12;

		if ((op + 1) > OPCODE_COUNT) {
			fprintf(stderr,
				"Wrong opcode %x at addr %x\n, aborting...\n",
				op, _REGS[R_PC]);
			abort();
		}

		INSTRHANDLER handler = instrhandlers[op];

		if (handler == NULL) {
			fprintf(stderr,
				"Opcode %x at addr %x\n is disabled, aborting...\n",
				op, _REGS[R_PC]);
			abort();
		}
		
		dbgprint("MAIN: at addr: %x | op = %x\n", addrBackup, op);

		(*handler)(nextInstruction);
	}

	// Shutting down a VM and restoring term settings back to normal
	// TODO: look it up
	restore_input_buffering();

	return 0;
}






























