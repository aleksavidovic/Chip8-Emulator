#include "debug.h"
#include <stdio.h>
#include <SDL2/SDL.h>
#include "chip8.h"
#include "config.h"

void render_memory(SDL_Renderer* renderer, uint8_t memory[]) {
	SDL_SetRenderDrawColor(renderer, 20, 20, 40, 255); // Dark blue background
    SDL_RenderClear(renderer);

    int bytes_per_row = 64;
    int cell_size = 8; // Each memory byte is an 8x8 pixel square

    // CHIP-8 has 4096 bytes of memory
    for (int i = 0; i < 4096; ++i) {
        int x = (i % bytes_per_row) * cell_size;
        int y = (i / bytes_per_row) * cell_size;

        // Get the value of the byte, determines the brightness
        uint8_t byte_value = memory[i];

        // Draw a rectangle for each byte, colored by its value
        SDL_Rect mem_cell = { x, y, cell_size, cell_size };
        SDL_SetRenderDrawColor(renderer, byte_value, byte_value, byte_value, 255);
        SDL_RenderFillRect(renderer, &mem_cell);
    }

    SDL_RenderPresent(renderer);}

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
