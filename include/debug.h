#ifndef DEBUG_H
#define DEBUG_H

#include "chip8.h"
#include "config.h"

#define FALLBACK_DUMP_FILENAME "~/dump.txt"

int dump_state(chip8_t* chip8, chip8_config* config, const char* dump_filename);

#endif // DEBUG_H
