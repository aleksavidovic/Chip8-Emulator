#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include "chip8.h"
#include <stdlib.h>
#include <time.h>

void chip8_initialize(chip8_t* chip8) {
    memset(chip8, 0, sizeof(chip8_t));
    chip8->pc = 0x200;
    srand(time(NULL));
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
    /*
        most opcodes follow one of the following patterns:
            - ?nnn
            - ?xy?
            - ?xkk
            - ?xkn
        For readability, we extract each of those values in the outer block
        and simply reference them inside cases.
    */
    uint8_t x = (opcode >> 8) & 0x000F;
    uint8_t y = (opcode >> 4) & 0x000F;
    uint8_t kk = opcode & 0x00FF;
    uint8_t n = opcode & 0x000F;
    uint8_t nnn = opcode & 0x0FFF;

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
            chip8->pc = nnn;
            break;
        }
        case 0x2000: { /// 2nnn -> CALL addr
            /*
                Call subroutine at nnn. The interpreter increments the stack pointer, 
                then puts the current PC on the top
                of the stack. The PC is then set to nnn.
            */
            chip8->stack_pointer++;
            chip8->stack[chip8->stack_pointer] = chip8->pc;
            chip8->pc = nnn;
            break;
        }
        case 0x3000: { // 3xkk -> SE Vx, byte
            /*
                Skip next instruction if Vx = kk. 
                The interpreter compares register Vx to kk, and if they are equal,
                increments the program counter by 2. 
            */
            if (chip8->v_registers[x] == kk )
                chip8->pc += 2;
            break;
        }
        case 0x4000: { // 4xkk -> SNE Vx, byte
            /*
                Skip next instruction if Vx != kk. 
                The interpreter compares register Vx to kk, and if they are not equal,
                increments the program counter by 2.
            */
            if (chip8->v_registers[x] != kk)
                chip8->pc += 2;
            break;
        }
        case 0x5000: { // 5xy0 -> SE Vx, Vy
            /*
                Skip next instruction if Vx = Vy. 
                The interpreter compares register Vx to register Vy, and if they are equal,
                increments the program counter by 2. 
            */
            if (chip8->v_registers[x] == chip8->v_registers[y])
                chip8->pc += 2;
            break;
        }
        case 0x6000: { // 6xkk -> LD Vx, byte
            /*
                Set Vx = kk. The interpreter puts the value kk into register Vx.
            */
            chip8->v_registers[x] = kk;
            break;
        }
        case 0x7000: { // 7xkk -> ADD Vx, byte
            chip8->v_registers[x] += kk;
            break;
        }
        case 0x8000: { // Group of operations 0x8xy0 : 0x8xyE
            switch (opcode & 0x000F) {
                case 0x0000: { // 8xy0 -> LD Vx, Vy
                    /*
                        Set Vx = Vy. Stores the value of register Vy in register Vx. 
                    */
                    chip8->v_registers[x] = chip8->v_registers[y];
                    break;
                }
                case 0x0001: { // 8xy1 -> OR Vx, Vy 
                    /*
                        Set Vx = Vx OR Vy. Performs a bitwise 
                        OR on the values of Vx and Vy, then stores the result in Vx. 
                        A bitwise OR compares the corresponding bits from two values, 
                        and if either bit is 1, then the same bit in the result is also 1. Otherwise, it is 0. 
                    */
                    chip8->v_registers[x] = chip8->v_registers[x] | chip8->v_registers[y];
                    break;
                }
                case 0x0002: { // 8xy2 -> AND Vx, Vy
                    /*
                        Set Vx = Vx AND Vy. Performs a bitwise AND on the values of 
                        Vx and Vy, then stores the result in Vx. A bitwise AND compares 
                        the corresponding bits from two values, and if both bits are 1, 
                        then the same bit in the result is also 1. Otherwise, it is 0.
                    */
                    chip8->v_registers[x] = chip8->v_registers[x] & chip8->v_registers[y];
                    break;
                }
                case 0x0003: { // 8xy3 -> XOR Vx, Vy
                    /*
                        Set Vx = Vx XOR Vy. Performs a bitwise exclusive OR on the 
                        values of Vx and Vy, then stores the result in Vx. An exclusive OR 
                        compares the corresponding bits from two values, and if the bits 
                        are not both the same, then the corresponding bit in the result 
                        is set to 1. Otherwise, it is 0. 
                    */
                    chip8->v_registers[x] = chip8->v_registers[x] ^ chip8->v_registers[y];
                    break;
                }
                case 0x0004: { // 8xy4 -> ADD Vx, Vy
                    /*
                        Set Vx = Vx + Vy, set VF = carry. The values of Vx and Vy are 
                        added together. If the result is greater than 8 bits (i.e., ¿ 255,) 
                        VF is set to 1, otherwise 0. Only the lowest 8 bits of the result 
                        are kept, and stored in Vx.
                    */
                    if ( UINT8_MAX - chip8->v_registers[x] > chip8->v_registers[y] )
                        chip8->v_registers[0xF] = 1;
                    else
                        chip8->v_registers[0xF] = 0;
                    chip8->v_registers[x] = chip8->v_registers[x] + chip8->v_registers[y];
                    break;
                }
                case 0x0005: { // 8xy5 -> SUB Vx, Vy
                    /*
                        Set Vx = Vx - Vy, set VF = NOT borrow. 
                        If Vx > Vy, then VF is set to 1, otherwise 0. 
                        Then Vy is subtracted from Vx, and the results stored in Vx. 
                    */
                    if (chip8->v_registers[x] > chip8->v_registers[y])
                        chip8->v_registers[0xF] = 1;
                    else
                        chip8->v_registers[0xF] = 0;
                    chip8->v_registers[x] = chip8->v_registers[x] - chip8->v_registers[y];
                    break;
                }
                case 0x0006: { // 8xy6 -> SHR Vx {, Vy}
                    /*
                        Set Vx = Vx SHR 1. If the least-significant bit of Vx is 1, 
                        then VF is set to 1, otherwise 0. Then Vx is divided by 2. 
                    */
                    chip8->v_registers[0xF] = (chip8->v_registers[x] & 0x0001 );
                    chip8->v_registers[x] >>= 1;
                    break;
                }
                case 0x0007: { // 8xy7 -> SUBN Vx, Vy
                    /*
                        Set Vx = Vy - Vx, set VF = NOT borrow. 
                        If Vy > Vx, then VF is set to 1, otherwise 0. 
                        Then Vx is subtracted from Vy, and the results stored in Vx.
                    */
                    chip8->v_registers[0xF] = (chip8->v_registers[y] > chip8->v_registers[x]) ? 1 : 0;
                    chip8->v_registers[x] = chip8->v_registers[y] - chip8->v_registers[x];
                    break;
                }
                case 0x000E: { // 8xyE -> SHL Vx {, Vy}
                    /*
                        Set Vx = Vx SHL 1. 
                        If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0. 
                        Then Vx is multiplied by 2.
                    */
                    chip8->v_registers[0xF] = (chip8->v_registers[x] >> 15) & 0x0001;
                    chip8->v_registers[x] <<= 1;
                    break;
                }
            }
            break;
        }
        case 0x9000: { // 9xy0 -> SNE Vx, Vy
            /*
                Skip next instruction if Vx != Vy. 
                The values of Vx and Vy are compared, and if they are not equal, 
                the program counter is increased by 2.
            */ 
            if ( chip8->v_registers[x] != chip8->v_registers[y] )
                chip8->pc += 2;
            break;
        }
        case 0xA000: { // Annn -> LD I, addr
            /*
                Set I = nnn. 
                The value of register I is set to nnn.
            */
            chip8->i_reg = nnn; 
            break;
        }
        case 0xB000: { // Bnnn -> JP V0, addr
            /*
                Jump to location nnn + V0. 
                The program counter is set to nnn plus the value of V0.
            */
            chip8->pc = chip8->v_registers[0x0] + nnn; 
            break;
        }
        case 0xC000: { // Cxkk -> RND Vx, byte 
            /*
                Set Vx = random byte AND kk. 
                The interpreter generates a random number from 0 to 255, which is then
                ANDed with the value kk. The results are stored in Vx. 
            */
            chip8->v_registers[x] = (uint8_t)(rand() % 256) & kk;
            break;
        }
        case 0xD000: { // Dxyn -> DRW Vx, Vy, nibble 
            /*
                Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision. 
                The interpreter reads n bytes from memory, starting at the address stored in I. 
                These bytes are then displayed as sprites on screen at coordinates (Vx, Vy). 
                Sprites are XOR’d onto the existing screen. If this causes any pixels to be erased,
                VF is set to 1, otherwise it is set to 0. If the sprite is positioned so part of it 
                is outside the coordinates of the display, it wraps around to the opposite side 
                of the screen.
            */
            uint8_t Vx = chip8->v_registers[x];
            uint8_t Vy = chip8->v_registers[y];
            uint16_t I = chip8->i_reg;
            chip8->v_registers[0xF] = 0u;
            for (int i = 0; i < (n * 8) ; i++) {
                if ( chip8->display[ ((Vy * 64) + Vx + i) ] == 1 )
                    chip8->v_registers[0xF] = 1u;
                chip8->display[ ((Vy * 64) + Vx + i) ] ^= chip8->memory[I+i];
            }            
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
