#include <stdio.h>
#include <stdint.h> 
#include <SDL2/SDL.h>
#include <stdbool.h>
#include "chip8.h"
#include "config.h"
#include "debug.h"

const int FPS = 60;
const int FRAME_DELAY = 1000 / FPS;
const int INSTRUCTIONS_PER_FRAME = 700 / FPS;

void render_graphics(SDL_Renderer *renderer, const uint8_t display[], uint8_t scale);
void handle_input(chip8_t* chip8, bool* running);

#define DUMP_FILENAME "dump.txt" 

int main(int argc, char *argv[]) {
    chip8_config config;
    if (parse_arguments(argc, argv, &config) != 0)
        return 1;
    
    printf("\nPress Enter to continue...");
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { };

    chip8_t chip8;
    chip8_initialize(&chip8, &config);
    chip8_load_rom(&chip8, config.rom_path);

    const int SCREEN_WIDTH = DISPLAY_WIDTH * config.scale_factor;
    const int SCREEN_HEIGHT = DISPLAY_HEIGHT * config.scale_factor;

    // SDL init
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("CHIP-8 Emulator", 
                                           SDL_WINDOWPOS_UNDEFINED, 
                                           SDL_WINDOWPOS_UNDEFINED, 
                                           SCREEN_WIDTH, 
                                           SCREEN_HEIGHT, 
                                           SDL_WINDOW_SHOWN);
    if (!window) {
        fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
        return 1;
    }   

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
        return 1;
    }
	
	// Create memory visualiser window
	// TODO: Make this work based on -v flag
	MemoryVisualiser_t mem_vis;
	int main_x, main_y, main_w, main_h;
	SDL_GetWindowPosition(window, &main_x, &main_y);
	SDL_GetWindowSize(window, &main_w, &main_h);
	if (!memory_visualiser_init(&mem_vis, (main_x + main_w)))
		printf("Memory visualiser initialisastion failed.\n");

    uint32_t last_timer_update = SDL_GetTicks();
    bool running = true;
	int64_t cycles_elapsed = 0;
	
    // --- Main emulation loop --- 
    while (running) {
        uint32_t frame_start = SDL_GetTicks();

		render_memory(mem_vis.renderer, chip8.memory);
		int sc;
		if (config.step_mode || config.cycles_to_run) cycles_elapsed++;
		if (config.step_mode) {
			printf("Emulation cycle: %15lu | Press <Enter> to continue. Type D to dump state and exit...\n", cycles_elapsed);
			while ((sc = getchar()) != '\n' && sc != EOF) {
				if (sc == 'D' || sc == 'd') {
					if(dump_state(&chip8, &config, DUMP_FILENAME))
						printf("Dump unsuccessful.\n");
					printf("Exiting...\n");
					return 0; 
				}
			};
		}
		if (config.cycles_to_run == cycles_elapsed) {
			printf("%ld cycles completed. Dump state before exiting? (Y/N) > ", cycles_elapsed);
			sc = getchar();
			if (sc == 'Y' || sc == 'y') {
				if(dump_state(&chip8, &config, DUMP_FILENAME))
					printf("Dump unsuccessful.\n");
				printf("Exiting...\n");
				return 0; 
			}
		}
			
        handle_input(&chip8, &running);
        for (int i = 0; i < INSTRUCTIONS_PER_FRAME; i++) {
            chip8_emulate_cycle(&chip8);
        }

        update_timers(&chip8, &last_timer_update);

        if (chip8.draw_flag) {
            render_graphics(renderer, chip8.display, config.scale_factor);
            chip8.draw_flag = false;
        }

        uint32_t frame_time = SDL_GetTicks() - frame_start;
        if (frame_time < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - frame_time);
        }
    }
    
    return 0;
}


void render_graphics(SDL_Renderer *renderer, const uint8_t display[], uint8_t scale) {
    /*
        1. Iterate through display array
            - Use two for loops for simpler scaled_x and scaled_y calc
        2. For each pixel, IF a pixel is "on" define a SDL_Rect (x, y, h=10, w=10)
        3. Tell the renderer to draw it
        (y * 64) + x
    */ 
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Set background color to black
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Set draw color to white

    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (display[(y * 64) + x] == 1) {
                SDL_Rect pixel_rect = {
                        .x = x * scale,
                        .y = y * scale,
                        .w = scale,
                        .h = scale 
                };
                SDL_RenderFillRect(renderer, &pixel_rect);
            } 
        }
    } 
    SDL_RenderPresent(renderer);
}

static const SDL_Keycode keymap[NUM_KEYS] = {
    SDLK_1, SDLK_2, SDLK_3, SDLK_4,
    SDLK_q, SDLK_w, SDLK_e, SDLK_r,
    SDLK_a, SDLK_s, SDLK_d, SDLK_f,
    SDLK_z, SDLK_x, SDLK_c, SDLK_v
};


void handle_input(chip8_t* chip8, bool* running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) *running = false;
        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            for (int i = 0; i < NUM_KEYS; i++) {
                if (event.key.keysym.sym == keymap[i]) {
                    chip8->keypad[i] = (event.type == SDL_KEYDOWN) ? 1 : 0;
                    break;
                }
            }
        }
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) *running = false;
    }
}