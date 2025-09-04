#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    const char *rom_path;
    bool step_mode;
    int64_t cycles_to_run;
    uint32_t clock_rate; 
    uint32_t scale_factor; 
    bool legacy_mode;
} chip8_config;

int parse_arguments(int argc, char *argv[], chip8_config *config);

#endif // CONFIG_H
