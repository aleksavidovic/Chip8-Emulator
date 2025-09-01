#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include "chip8.h"

void chip8_initialize(chip8_t* chip8) {
    memset(chip8, 0, sizeof(chip8_t));
    chip8->pc = 0x200;
}

void chip8_load_rom(chip8_t* chip8, const char* filename) {
    FILE* rom_file = fopen(filename, "rb");
    if (!rom_file) {
        fprintf(stderr, "Error: Could not open ROM file %s\n", filename);
        return;
    }
    
    fseek(rom_file, 0, SEEK_END);
    long rom_size = ftell(rom_file);
    fseek(rom_file, 0, SEEK_SET);

    if (rom_size > (4096 - 0x200)) {
        fprintf(stderr, "Error: ROM file is too large!\n");
    } else {
        fread(&chip8->memory[0x200], 1, rom_size, rom_file);
        printf("Loaded %ld bytes from %s\n", rom_size, filename);
    }

    fclose(rom_file);
}

void chip8_emulate_cycle(chip8_t* chip8) { // Fetch->Decode->Execute opcodes 
    uint16_t opcode = (chip8->memory[chip8->pc] << 8) | chip8->memory[chip8->pc+1];
    chip8->pc += 2;

    switch (opcode & 0xF000) { // get opcode "family"
        case 0x0000: { // CLS and RET
            switch (opcode) {
                case 0x00E0: { // CLS 
                    memset(chip8->display, 0u, sizeof(chip8->display));
                    break;
                }
                case 0x00EE: { // RET
                    // implement RET logic here
                    break;
                }
            }
            break;
        }
        case 0x1000: { // 1nnn -> JP addr
            uint16_t addr = opcode & 0x0FFF;
            chip8->pc = addr;
            break;
        }
        case 0x6000: { // 6xkk -> LD Vx, byte
            uint8_t reg_num = (opcode >> 8) & 0x000F;
            uint16_t value = opcode & 0x00FF;
            chip8->v_registers[reg_num] = value;
            break;
        }
        case 0x7000: { // 7xkk -> ADD Vx, byte
            uint8_t reg_num = (opcode >> 8) & 0x000F;
            uint16_t value = opcode & 0x00FF;
            chip8->v_registers[reg_num] += value;
            break;
        }
    }
}

void update_timers(chip8_t* chip8, uint32_t* last_timer_update) {
    uint32_t current_time = SDL_GetTicks();
    if (current_time - *last_timer_update >= (1000 / 60)) {
        if (chip8->delay_timer > 0) {
            chip8->delay_timer--;
        }
        if (chip8->sound_timer > 0) {
            chip8->sound_timer--;
            if (chip8->sound_timer == 0) {
                printf("BEEP!\n");
            }
        }
        *last_timer_update = current_time;
    }
}
