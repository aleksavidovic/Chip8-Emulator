#include <stdio.h>
#include "chip8.h"
#include "config.h"

#define FALLBACK_DUMP_FILENAME "~/dump.txt"

int dump_state(chip8_t* chip8, chip8_config* config, const char* dump_filename) {
	printf("Dumping state of the emulator to: %s\n", dump_filename);	
	return 0;
}
