#ifndef DEBUG_H
#define DEBUG_H

#include "chip8.h"
#include "config.h"
#define MEM_VIS_SCREEN_WIDTH  100
#define MEM_VIS_SCREEN_HEIGHT 100

typedef struct {
	SDL_Window* window;
	SDL_Renderer* renderer;
} MemoryVisualiser_t;

#define FALLBACK_DUMP_FILENAME "~/dump.txt"

int dump_state(chip8_t* chip8, chip8_config* config, const char* dump_filename);
int memory_visualiser_init(MemoryVisualiser_t* mem_vis);

#endif // DEBUG_H
