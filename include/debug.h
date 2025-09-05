#ifndef DEBUG_H
#define DEBUG_H

#include "chip8.h"
#include "config.h"
#include <SDL2/SDL.h>
#define MEM_VIS_SCREEN_WIDTH  512 
#define MEM_VIS_SCREEN_HEIGHT 256

typedef struct {
	SDL_Window* window;
	SDL_Renderer* renderer;
} MemoryVisualiser_t;

#define FALLBACK_DUMP_FILENAME "~/dump.txt"

int dump_state(chip8_t* chip8, chip8_config* config, const char* dump_filename);
int memory_visualiser_init(MemoryVisualiser_t* mem_vis, int x);
void render_memory(SDL_Renderer* renderer, uint8_t memory[]);

#endif // DEBUG_H
