#include <stdio.h>
#include <stdint.h> 
#include <SDL2/SDL.h>
#include <stdbool.h>
#include "chip8.h"
#include "config.h"
#include "debug.h"

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

    // Main emulation loop
    uint32_t last_timer_update = SDL_GetTicks();
    bool running = true;
	int64_t cycles_elapsed = 0;
    while (running) {
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

        chip8_emulate_cycle(&chip8);

        update_timers(&chip8, &last_timer_update);

        if (chip8.draw_flag) {
            render_graphics(renderer, chip8.display, config.scale_factor);
            chip8.draw_flag = false;
        }

        SDL_Delay(1);
    }
    
    return 0;
}


void render_graphics(SDL_Renderer *renderer, const uint8_t display[], uint8_t scale) {
    /*
        1. Iterate through display array
            - Use two for loops for simpler scaled_x and scaled_y calc
        2. For each pixel, IF a pixel is "on" define a SDL_Rect (x, y, h=10, w=10)
        3. Tell the renderer to draw it
        (x * 64) + y
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


void handle_input(chip8_t* chip8, bool* running) {
    // Handle input
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                *running = false;
                break;
            
            case SDL_KEYDOWN: {
                // A key was pressed
                SDL_Keycode key = event.key.keysym.sym;
                printf("-------Keypress detected-----------\n");
                switch (key) {
                    case SDLK_1: chip8->keypad[0x1] = 1; break;
                    case SDLK_2: chip8->keypad[0x2] = 1; break;
                    case SDLK_3: chip8->keypad[0x3] = 1; break;
                    case SDLK_4: chip8->keypad[0xC] = 1; break;
                    case SDLK_q: chip8->keypad[0x4] = 1; break;
                    case SDLK_w: chip8->keypad[0x5] = 1; break;
                    case SDLK_e: chip8->keypad[0x6] = 1; break;
                    case SDLK_r: chip8->keypad[0xD] = 1; break;
                    case SDLK_a: chip8->keypad[0x7] = 1; break;
                    case SDLK_s: chip8->keypad[0x8] = 1; break;
                    case SDLK_d: chip8->keypad[0x9] = 1; break;
                    case SDLK_f: chip8->keypad[0xE] = 1; break;
                    case SDLK_z: chip8->keypad[0xA] = 1; break;
                    case SDLK_x: chip8->keypad[0x0] = 1; break;
                    case SDLK_c: chip8->keypad[0xB] = 1; break;
                    case SDLK_v: chip8->keypad[0xF] = 1; break;
                }
                break;
            }

            case SDL_KEYUP: { 
                // A key was released
                SDL_Keycode key = event.key.keysym.sym;
                switch (key) {
                    case SDLK_1: chip8->keypad[0x1] = 0; break;
                    case SDLK_2: chip8->keypad[0x2] = 0; break;
                    case SDLK_3: chip8->keypad[0x3] = 0; break;
                    case SDLK_4: chip8->keypad[0xC] = 0; break;
                    case SDLK_q: chip8->keypad[0x4] = 0; break;
                    case SDLK_w: chip8->keypad[0x5] = 0; break;
                    case SDLK_e: chip8->keypad[0x6] = 0; break;
                    case SDLK_r: chip8->keypad[0xD] = 0; break;
                    case SDLK_a: chip8->keypad[0x7] = 0; break;
                    case SDLK_s: chip8->keypad[0x8] = 0; break;
                    case SDLK_d: chip8->keypad[0x9] = 0; break;
                    case SDLK_f: chip8->keypad[0xE] = 0; break;
                    case SDLK_z: chip8->keypad[0xA] = 0; break;
                    case SDLK_x: chip8->keypad[0x0] = 0; break;
                    case SDLK_c: chip8->keypad[0xB] = 0; break;
                    case SDLK_v: chip8->keypad[0xF] = 0; break;
                }
                break;
            }
        }
    }
}

