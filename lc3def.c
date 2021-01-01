// FILE lc3def.c

#include "lc3def.h"

void dbgprint(const char* format, ...) {
#ifdef DEBUG_TRACE_ON
	
	va_list arguments; 
	va_start(arguments, format);

	FILE *logFile = fopen("lc3vm.log", "a");

	if (!logFile) {
		fprintf(stderr,
			"Unable to open a log file, aborting...");

		abort();
	}

	vfprintf(logFile, format, arguments);
	fclose(logFile);

	va_end(arguments);

#endif
}

UINT check_key() {
	fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    return select(1, &readfds, NULL, NULL, &timeout) != 0;
}

void disable_input_buffering() {
	tcgetattr(STDIN_FILENO, &original_tio);
    struct termios new_tio = original_tio;
    new_tio.c_lflag &= ~ICANON & ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

void restore_input_buffering() {
	tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
}

void handle_interrupt(int signal) {
	restore_input_buffering();
    printf("\n");
    exit(-2);
}

/*
 * Reading lc-3 assembly image file
 */

UINT readImageFile(const char *imagePath) {
	FILE* img = fopen(imagePath, "rb");

	if (!img) {
		fprintf(stderr, 
			"Error: unable to find specified file \"%s\"\n", 
			imagePath);

		return 0;
	}
	
	UINT startOffset;
	fread(&startOffset, sizeof(UINT), 1, img);
	startOffset = (startOffset << 8) | (startOffset >> 8); // big-endian to little-endian
	
	UINT imgsize = MAX_MEM - startOffset;
	UINT *memWritePtr = _MEM + startOffset;
	size_t readCount = fread(memWritePtr, sizeof(UINT), imgsize, img);

	while (readCount > 0) {
		*memWritePtr = (*memWritePtr << 8) | (*memWritePtr >> 8); // big-endian to little endian again
		memWritePtr += 1;
		readCount -= 1;
	}

	fclose(img);
	return 1;
}



































