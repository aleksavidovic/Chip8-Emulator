#include <stdio.h>
#include <SDL2/SDL.h>
#include "chip8.h"
#include "config.h"
#include "debug.h"

int memory_visualiser_init(MemoryVisualiser_t* mem_vis, int x) {
    SDL_Window* mem_vis_window = SDL_CreateWindow("Memory Visualiser", 
                                           x, 
                                           SDL_WINDOWPOS_UNDEFINED, 
                                           MEM_VIS_SCREEN_WIDTH, 
                                           MEM_VIS_SCREEN_HEIGHT, 
                                           SDL_WINDOW_SHOWN);

    if (!mem_vis_window) {
        fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
        return 1;
    }   
	SDL_Renderer* mem_vis_renderer = SDL_CreateRenderer(mem_vis_window,
														-1,
														SDL_RENDERER_ACCELERATED);

    if (!mem_vis_renderer) {
        fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
        return 1;
    }
	mem_vis->window = mem_vis_window;
	mem_vis->renderer = mem_vis_renderer;
	return 0;
}

int dump_state(chip8_t* chip8, chip8_config* config, const char* dump_filename) {
	printf("Dumping state of the emulator to: %s\n", dump_filename);	
	print_emulator_configuration(config);
	return 0;
}
