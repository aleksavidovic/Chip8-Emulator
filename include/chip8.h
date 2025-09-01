#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <stdbool.h>

#define MEMORY_SIZE 4096
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define NUM_REGISTERS 16
#define STACK_LEVELS 16
#define NUM_KEYS 16

typedef struct {
    uint8_t memory[MEMORY_SIZE];
    uint8_t display[DISPLAY_WIDTH * DISPLAY_HEIGHT];
    uint16_t pc;
    uint16_t i_reg;
    uint16_t stack[STACK_LEVELS];
    uint8_t stack_pointer;
    uint8_t v_registers[NUM_REGISTERS];
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint8_t keypad[NUM_KEYS];
    bool draw_flag;
} chip8_t;

void chip8_initialize(chip8_t* chip8);
void chip8_load_rom(chip8_t* chip8, const char* filename);
void chip8_emulate_cycle(chip8_t* chip8);
void update_timers(chip8_t* chip8, uint32_t* last_timer_update);

#endif
