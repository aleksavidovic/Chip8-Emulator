#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include "chip8.h"
#include <stdlib.h>
#include <time.h>

typedef bool (*opcode_func_t)(chip8_t* chip8, uint16_t opcode);

static const uint8_t chip8_font_set[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void chip8_initialize(chip8_t* chip8) {
    memset(chip8, 0, sizeof(chip8_t));
    chip8->pc = 0x200;
    srand(time(NULL));
    memcpy(&chip8->memory[FONT_START_ADDRESS], chip8_font_set, sizeof(chip8_font_set));
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

static inline uint8_t get_x(uint16_t opcode) {
    return (opcode >> 8) & 0x0F;
}

static inline uint8_t get_y(uint16_t opcode) {
    return (opcode >> 4) & 0x0F;
}

static inline uint8_t get_kk(uint16_t opcode) {
    return (opcode & 0xFF);
}

static inline uint8_t get_n(uint16_t opcode) {
    return (opcode & 0x0F);
}

static inline uint16_t get_nnn(uint16_t opcode) {
    return (opcode & 0x0FFF);
}


bool op_unknown(chip8_t* chip8, uint16_t opcode) {
    fprintf(stderr, "Unknown opcode: 0x%04X\n", opcode);
    return false;
}

bool op_0xxx(chip8_t* chip8, uint16_t opcode);

bool op_00E0(chip8_t* chip8, uint16_t opcode) {
    memset(chip8->display, 0, sizeof(chip8->display));
    return false;
}

bool op_00EE(chip8_t* chip8, uint16_t opcode) {
    chip8->pc = chip8->stack[--chip8->stack_pointer];
    return true;
}

bool op_1nnn(chip8_t* chip8, uint16_t opcode) { // 1nnn -> JP addr
    chip8->pc = get_nnn(opcode);
    return true;
}

bool op_2nnn(chip8_t* chip8, uint16_t opcode) { // 2nnn -> CALL addr
    /*
        Call subroutine at nnn. The interpreter increments the stack pointer, 
        then puts the current PC on the top
        of the stack. The PC is then set to nnn.
    */
    chip8->stack[chip8->stack_pointer++] = chip8->pc;
    chip8->pc = get_nnn(opcode);
    return true;
}

bool op_3xkk(chip8_t* chip8, uint16_t opcode) { // 3xkk -> SE Vx, byte
    /*
        Skip next instruction if Vx = kk. 
        The interpreter compares register Vx to kk, and if they are equal,
        increments the program counter by 2. 
    */
    if (chip8->V[get_x(opcode)] == get_kk(opcode))
        chip8->pc += 2;
    return false;
}

bool op_4xkk(chip8_t* chip8, uint16_t opcode) { // 4xkk -> SNE Vx, byte
    /*
        Skip next instruction if Vx != kk. 
        The interpreter compares register Vx to kk, and if they are not equal,
        increments the program counter by 2.
    */
    if (chip8->V[get_x(opcode)] != get_kk(opcode))
        chip8->pc += 2;
    return false;
}

bool op_5xy0(chip8_t* chip8, uint16_t opcode) { // 5xy0 -> SE Vx, Vy
    /*
        Skip next instruction if Vx = Vy. 
        The interpreter compares register Vx to register Vy, and if they are equal,
        increments the program counter by 2. 
    */
    if (chip8->V[get_x(opcode)] == chip8->V[get_y(opcode)])
        chip8->pc += 2;
    return false;
}

bool op_6xkk(chip8_t* chip8, uint16_t opcode) { // 6xkk -> LD Vx, byte
    /*
        Set Vx = kk. The interpreter puts the value kk into register Vx.
    */
    chip8->V[get_x(opcode)] = get_kk(opcode);
    return false;
}

bool op_7xkk(chip8_t* chip8, uint16_t opcode) { // 7xkk -> ADD Vx, byte
    /*
        Set Vx = Vx + kk.
    */
    chip8->V[get_x(opcode)] += get_kk(opcode);
    return false;
}

bool op_8xxx(chip8_t* chip8, uint16_t opcode);


bool op_8xy0(chip8_t* chip8, uint16_t opcode) { // 8xy0 -> LD Vx, Vy
    /*
        Set Vx = Vy. 
        Stores the value of register Vy in register Vx. 
    */
    chip8->V[get_x(opcode)] = chip8->V[get_y(opcode)];
    return false;
}

bool op_8xy1(chip8_t* chip8, uint16_t opcode) { // 8xy1 -> OR Vx, Vy 
    /*
        Set Vx = Vx OR Vy. Performs a bitwise 
        OR on the values of Vx and Vy, then stores the result in Vx. 
        A bitwise OR compares the corresponding bits from two values, 
        and if either bit is 1, then the same bit in the result is also 1. Otherwise, it is 0. 
    */
    chip8->V[get_x(opcode)] |= chip8->V[get_y(opcode)];
    return false;
}

bool op_8xy2(chip8_t* chip8, uint16_t opcode) { // 8xy2 -> AND Vx, Vy
    /*
        Set Vx = Vx AND Vy. 
        Performs a bitwise AND on the values of Vx and Vy, 
        then stores the result in Vx. A bitwise AND compares 
        the corresponding bits from two values, and if both bits are 1, 
        then the same bit in the result is also 1. Otherwise, it is 0.
    */
    chip8->V[get_x(opcode)] &= chip8->V[get_y(opcode)];
    return false;
}

bool op_8xy3(chip8_t* chip8, uint16_t opcode) { // 8xy3 -> XOR Vx, Vy
    /*
        Set Vx = Vx XOR Vy. 
        Performs a bitwise exclusive OR on the values of Vx and Vy, 
        then stores the result in Vx. An exclusive OR 
        compares the corresponding bits from two values, and if the bits 
        are not both the same, then the corresponding bit in the result 
        is set to 1. Otherwise, it is 0. 
    */
    chip8->V[get_x(opcode)] ^= chip8->V[get_y(opcode)];
    return false;
}

bool op_8xy4(chip8_t* chip8, uint16_t opcode) { // 8xy4 -> ADD Vx, Vy
    /* 
        Set Vx = Vx + Vy, set VF = carry. 
        The values of Vx and Vy are added together. 
        If the result is greater than 8 bits (i.e., ¿ 255,) 
        VF is set to 1, otherwise 0. Only the lowest 8 bits of the result 
        are kept, and stored in Vx.
    */
    uint8_t x = get_x(opcode);
    uint8_t y = get_y(opcode);
    uint16_t sum = chip8->V[x] + chip8->V[y];
    chip8->V[0xF] = (sum > 255) ? 1 : 0;
    chip8->V[x] = (uint8_t)sum;
    return false;
}

bool op_8xy5(chip8_t* chip8, uint16_t opcode) { // 8xy5 -> SUB Vx, Vy
    /*
        Set Vx = Vx - Vy, set VF = NOT borrow. 
        If Vx > Vy, then VF is set to 1, otherwise 0. 
        Then Vy is subtracted from Vx, and the results stored in Vx. 
    */
    uint8_t x = get_x(opcode);
    uint8_t y = get_y(opcode);
    chip8->V[0xF] = (chip8->V[x] > chip8->V[y]) ? 1 : 0;
    chip8->V[x] -= chip8->V[y];
    return false;
}

bool op_8xy6(chip8_t* chip8, uint16_t opcode) { // 8xy6 - SHR Vx {, Vy}
    /*
        Set Vx = Vx SHR 1. 
        If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. 
        Then Vx is divided by 2.
    */
    uint8_t x = get_x(opcode);
    chip8->V[0xF] = (chip8->V[x] & 0x0001) ? 1 : 0; 
    chip8->V[x] >>= 1;
    return false;
}

bool op_8xy7(chip8_t* chip8, uint16_t opcode) { // 8xy7 -> SUBN Vx, Vy
    /*
        Set Vx = Vy - Vx, set VF = NOT borrow. 
        If Vy > Vx, then VF is set to 1, otherwise 0. 
        Then Vx is subtracted from Vy, and the results stored in Vx.
    */
    uint8_t x = get_x(opcode);
    uint8_t y = get_y(opcode);
    chip8->V[0xF] = (chip8->V[y] > chip8->V[x]) ? 1 : 0;
    chip8->V[x] = chip8->V[y] - chip8->V[x];
    return false;
}

bool op_8xyE(chip8_t* chip8, uint16_t opcode) { // 8xyE -> SHL Vx {, Vy}
    /*
        Set Vx = Vx SHL 1. 
        If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0. 
        Then Vx is multiplied by 2.
    */
    uint8_t x = get_x(opcode);
    chip8->V[0xF] = (chip8->V[x] & 0x80) >> 7;
    chip8->V[x] <<= 1;
    return false;
}

bool op_9xy0(chip8_t* chip8, uint16_t opcode) { // 9xy0 -> SNE Vx, Vy
    /*
        Skip next instruction if Vx != Vy. 
        The values of Vx and Vy are compared, and if they are not equal, 
        the program counter is increased by 2.
    */ 
    if ( chip8->V[get_x(opcode)] != chip8->V[get_y(opcode)] )
        chip8->pc += 2;
    return false;
}

bool op_Annn(chip8_t* chip8, uint16_t opcode) { // Annn -> LD I, addr
    /*
        Set I = nnn. 
        The value of register I is set to nnn.
    */
    chip8->I = get_nnn(opcode);
    return false;
}

bool op_Bnnn(chip8_t* chip8, uint16_t opcode) { // Bnnn -> JP V0, addr
    /*
        Jump to location nnn + V0. 
        The program counter is set to nnn plus the value of V0.
    */
    chip8->pc = chip8->V[0x0] + get_nnn(opcode);
    return true;
}

bool op_Cxkk(chip8_t* chip8, uint16_t opcode) { // Cxkk -> RND Vx, byte 
    /*
        Set Vx = random byte AND kk. 
        The interpreter generates a random number from 0 to 255, which is then
        ANDed with the value kk. The results are stored in Vx. 
    */
    chip8->V[get_x(opcode)] = ((uint8_t)(rand() % 256) & get_kk(opcode));
    return false;
}

bool op_Dxyn(chip8_t* chip8, uint16_t opcode) { // Dxyn -> DRW Vx, Vy, nibble 
    /*
        Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision. 
        The interpreter reads n bytes from memory, starting at the address stored in I. 
        These bytes are then displayed as sprites on screen at coordinates (Vx, Vy). 
        Sprites are XOR’d onto the existing screen. If this causes any pixels to be erased,
        VF is set to 1, otherwise it is set to 0. If the sprite is positioned so part of it 
        is outside the coordinates of the display, it wraps around to the opposite side 
        of the screen.
    */
    // Get the coordinates and height from the opcode
    uint8_t x_coord_reg = get_x(opcode);
    uint8_t y_coord_reg = get_y(opcode);
    uint8_t height = get_n(opcode);

    // Get the starting x and y from the V registers
    uint8_t start_x = chip8->V[x_coord_reg];
    uint8_t start_y = chip8->V[y_coord_reg];

    // Reset the collision flag
    chip8->V[0xF] = 0;

    // Loop over each row of the sprite (n rows)
    for (int y_line = 0; y_line < height; y_line++) {
        // Get the byte of sprite data for the current row
        uint8_t sprite_byte = chip8->memory[chip8->I + y_line];

        // Loop over each bit of the sprite byte (8 bits wide)
        for (int x_bit = 0; x_bit < 8; x_bit++) {
            // Check if the current sprite bit is set to 1
            // (0x80 is 10000000 in binary. We shift it right to check each bit)
            if ((sprite_byte & (0x80 >> x_bit)) != 0) {
                // Calculate the target screen coordinates, with wrapping
                int screen_x = (start_x + x_bit) % DISPLAY_WIDTH;
                int screen_y = (start_y + y_line) % DISPLAY_HEIGHT;

                // Convert 2D screen coords to 1D display array index
                int pixel_index = screen_x + (screen_y * DISPLAY_WIDTH);

                // Check for collision: if the screen pixel is already 1, set VF flag
                if (chip8->display[pixel_index] == 1) {
                    chip8->V[0xF] = 1;
                }

                // XOR the pixel onto the display buffer
                chip8->display[pixel_index] ^= 1;
            }
        }
    }

    // Set the draw flag so the screen updates in the main loop
    chip8->draw_flag = true;

    // This is not a jump/call instruction
    return false;
}

bool op_Exxx(chip8_t* chip8, uint16_t opcode);

bool op_Ex9E(chip8_t* chip8, uint16_t opcode) { // SKP Vx
    /*
        Skip next instruction if key with the value of Vx is pressed. 
        Checks the keyboard, and if the key corresponding to the value 
        of Vx is currently in the down position, PC is increased by 2. 
    */
    uint8_t x = get_x(opcode);
    uint8_t key_value = chip8->V[x]; 
    if (chip8->keypad[chip8->V[key_value]] == 1)
        chip8->pc += 2;
    return false;
}

bool op_ExA1(chip8_t* chip8, uint16_t opcode) { // SKNP Vx
    /*
        Skip next instruction if key with the value of Vx is not pressed. 
        Checks the keyboard, and if the key corresponding to the value 
        of Vx is currently in the up position, PC is increased by 2.
    */
    uint8_t x = get_x(opcode);
    uint8_t key_value = chip8->V[x]; 
    if (chip8->keypad[chip8->V[key_value]] == 0)
        chip8->pc += 2;
    return false;
}

bool op_Fxxx(chip8_t* chip8, uint16_t opcode);

bool op_Fx07(chip8_t* chip8, uint16_t opcode) { // Fx07 -> LD Vx, DT
    /*
        Set Vx = delay timer value. 
        The value of DT is placed into Vx.
    */
    chip8->V[get_x(opcode)] = chip8->delay_timer;
    return false;
}

bool op_Fx0A(chip8_t* chip8, uint16_t opcode) { // Fx0A -> LD Vx, K 
    /*
        Wait for a key press, store the value of the key in Vx. 
        All execution stops until a key is pressed, 
        then the value of that key is stored in Vx.
    */
    uint8_t x = get_x(opcode);
    for (int i = 0; i < NUM_KEYS; i++) {
        if (chip8->keypad[i] == 1) {
            chip8->V[x] = i;
            return false;
        }
    }
    return true;
}

bool op_Fx15(chip8_t* chip8, uint16_t opcode) { // Fx15 -> LD DT, Vx
    /*
        Set delay timer = Vx. 
        Delay Timer is set equal to the value of Vx.
    */
    chip8->delay_timer = chip8->V[get_x(opcode)];
    return false;
}

bool op_Fx18(chip8_t* chip8, uint16_t opcode) { // Fx18 -> LD ST, Vx
    /*
        Set sound timer = Vx. 
        Sound Timer is set equal to the value of Vx.
    */
    chip8->sound_timer = chip8->V[get_x(opcode)];
    return false;
}

bool op_Fx1E(chip8_t* chip8, uint16_t opcode) { // Fx1E -> ADD I, Vx
    /*
        Set I = I + Vx. 
        The values of I and Vx are added, and the results are stored in I.
    */
    chip8->I += chip8->V[get_x(opcode)];
    return false;
}

bool op_Fx29(chip8_t* chip8, uint16_t opcode) { // Fx29 -> LD F, Vx
    /*
        Set I = location of sprite for digit Vx. 
        The value of I is set to the location for the hexadecimal sprite
        corresponding to the value of Vx. 
    */
    chip8->I = FONT_START_ADDRESS + (chip8->V[get_x(opcode)] * 5);
    return false;
}

bool op_Fx33(chip8_t* chip8, uint16_t opcode) { // Fx33 -> LD B, Vx
    /*
        Store BCD representation of Vx in memory locations I, I+1, and I+2. 
        The interpreter takes the decimal value of Vx, and places the 
        hundreds digit in memory at location in I, 
        the tens digit at location I+1, and
        the ones digit at location I+2.
    */
    uint8_t Vx = chip8->V[get_x(opcode)];
    uint16_t I = chip8->I;
    chip8->memory[I] = Vx / 100 ;
    chip8->memory[I + 1] = (Vx / 10) % 10;
    chip8->memory[I + 2] = Vx % 10;
    return false;
}

bool op_Fx55(chip8_t* chip8, uint16_t opcode) { // Fx55 -> LD [I], Vx
    /*
        Stores V0 to VX in memory starting at address I. 
    */
    uint8_t x = get_x(opcode);
    for (int i = 0; i <= x; i++)
        chip8->memory[chip8->I + i] = chip8->V[i];
    return false;
}

bool op_Fx65(chip8_t* chip8, uint16_t opcode) { // Fx65 -> LD Vx, [I]
    /*
        Fills V0 to VX with values from memory starting at address I. 
    */
    uint8_t x = get_x(opcode);
    for (int i = 0; i <= x ; i++)
        chip8->V[i] = chip8->memory[chip8->I + i];
    return false;
}
/*
    All opcodes follow one of the following patterns:
        - ?nnn
        - ?xy?
        - ?xkk
        - ?xkn
*/

static opcode_func_t opcode_table[16] = {
    [0x0] = op_0xxx,  // Special case for 00E0 and 00EE
    [0x1] = op_1nnn,
    [0x2] = op_2nnn,
    [0x3] = op_3xkk,
    [0x4] = op_4xkk,
    [0x5] = op_5xy0,
    [0x6] = op_6xkk,
    [0x7] = op_7xkk,
    [0x8] = op_8xxx,  // Special case for arithmetic
    [0x9] = op_9xy0,
    [0xA] = op_Annn,
    [0xB] = op_Bnnn,
    [0xC] = op_Cxkk,
    [0xD] = op_Dxyn,
    [0xE] = op_Exxx,  // Special case for key ops
    [0xF] = op_Fxxx   // Special case for misc ops
};

static opcode_func_t opcode_0xxx_table[0xEF] = {
    [0xE0] = op_00E0,
    [0xEE] = op_00EE
};

static opcode_func_t opcode_8xxx_table[] = {
    [0x0] = op_8xy0,
    [0x1] = op_8xy1,
    [0x2] = op_8xy2,
    [0x3] = op_8xy3,
    [0x4] = op_8xy4,
    [0x5] = op_8xy5,
    [0x6] = op_8xy6,
    [0x7] = op_8xy7,
    [0xE] = op_8xyE
};

static opcode_func_t opcode_Exxx_table[] = {
    [0x9E] = op_Ex9E,
    [0xA1] = op_ExA1
};

static opcode_func_t opcode_Fxxx_table[] = {
    [0x07] = op_Fx07,
    [0x0A] = op_Fx0A,
    [0x15] = op_Fx15,
    [0x18] = op_Fx18,
    [0x1E] = op_Fx1E,
    [0x29] = op_Fx29,
    [0x33] = op_Fx33,
    [0x55] = op_Fx55,
    [0x65] = op_Fx65
};

bool op_0xxx(chip8_t* chip8, uint16_t opcode) {
    uint8_t kk = get_kk(opcode);

    // 1. Check if the index is within the array bounds.
    if (kk >= sizeof(opcode_0xxx_table) / sizeof(opcode_func_t)) {
        return op_unknown(chip8, opcode);
    }

    // 2. Get the function pointer.
    opcode_func_t func = opcode_0xxx_table[kk];

    // 3. Check if a function was actually defined for this opcode.
    if (func) {
        return func(chip8, opcode);
    } else {
        return op_unknown(chip8, opcode);
    }
}

bool op_Exxx(chip8_t* chip8, uint16_t opcode) {
    uint8_t kk = get_kk(opcode);
    if (kk >= sizeof(opcode_Exxx_table) / sizeof(opcode_func_t)) {
        return op_unknown(chip8, opcode);
    }
    opcode_func_t func = opcode_Exxx_table[kk];
    if (func) {
        return func(chip8, opcode);
    }
    return op_unknown(chip8, opcode);
}

bool op_8xxx(chip8_t* chip8, uint16_t opcode) {
    uint8_t n = get_n(opcode);
    if (n >= sizeof(opcode_8xxx_table) / sizeof(opcode_func_t)) {
        return op_unknown(chip8, opcode);
    }
    opcode_func_t func = opcode_8xxx_table[n];
    if (func) {
        return func(chip8, opcode);
    }
    return op_unknown(chip8, opcode);
}

bool op_Fxxx(chip8_t* chip8, uint16_t opcode) {
        uint8_t kk = get_kk(opcode);
    if (kk >= sizeof(opcode_Fxxx_table) / sizeof(opcode_func_t)) {
        return op_unknown(chip8, opcode);
    }
    opcode_func_t func = opcode_Fxxx_table[kk];
    if (func) {
        return func(chip8, opcode);
    }
    return op_unknown(chip8, opcode);
}

void chip8_emulate_cycle(chip8_t* chip8) { // Fetch->Decode->Execute opcodes 
    log_state(chip8);
    uint16_t opcode = (chip8->memory[chip8->pc] << 8) | chip8->memory[chip8->pc+1];

    uint8_t opcode_op = ((opcode >> 12) & 0x0F);
    opcode_func_t func = opcode_table[opcode_op];
    if (!func(chip8, opcode))
        chip8->pc += 2;
    
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

void log_state(chip8_t* chip8) {
    uint16_t opcode = (chip8->memory[chip8->pc] << 8) | chip8->memory[chip8->pc+1];
    printf("[0x%4X] 0x%4X | V0-VF[", chip8->pc, opcode);
    for (int i = 0; i < NUM_REGISTERS; i++)
        printf("%02X ", chip8->V[i]);
    printf("]\n");
}
