#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>

static void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s [options] <rom_path>\n", prog_name);
    fprintf(stderr, "\nOptions:\n");
    fprintf(stderr, "  -h, --help            Show this help message and exit\n");
    fprintf(stderr, "  -s, --step            Enable step-through mode (press Enter for each cycle)\n");
    fprintf(stderr, "  -l, --legacy          Enable legacy opcode behavior around I register\n");
    fprintf(stderr, "  -c, --cycles <count>  Run for a specific number of cycles and exit\n");
    fprintf(stderr, "  -r, --clock-rate <hz> Set the CPU clock speed in Hertz (default: 500)\n");
    fprintf(stderr, "  -S, --scale <factor>  Set the display scale factor (default: 10)\n");
}

static int parse_int64(const char *str, int64_t *val) {
    char *endptr;
    errno = 0;
    *val = strtoll(str, &endptr, 10);

    if (endptr == str || *endptr != '\0' || errno == ERANGE) {
        return -1;
    }
    return 0;
}

int parse_arguments(int argc, char *argv[], chip8_config *config) {
    
    // Set default values
    config->rom_path = NULL;
    config->step_mode = false;
    config->cycles_to_run = -1;
    config->clock_rate = 500;
    config->scale_factor = 10;
    config->legacy_mode = false;
    
    static struct option long_options[] = {
        {"help",       no_argument,       0, 'h'},
        {"step",       no_argument,       0, 's'},
        {"legacy",     no_argument,       0, 'l'},
        {"cycles",     required_argument, 0, 'c'},
        {"clock-rate", required_argument, 0, 'r'},
        {"scale",      required_argument, 0, 'S'},
        {0, 0, 0, 0}
    };
    const char *short_opts = "hslc:r:S:";

    // 3. The parsing loop
    int opt_char;
    while ((opt_char = getopt_long(argc, argv, short_opts, long_options, NULL)) != -1) {
        switch (opt_char) {
            case 'h': print_usage(argv[0]); exit(0);
            case 's': config->step_mode = true; break;
            case 'l': config->legacy_mode = true; break;
            case 'c':
                if (parse_int64(optarg, &config->cycles_to_run) != 0 || config->cycles_to_run <= 0) {
                    fprintf(stderr, "Error: Invalid number for cycles: '%s'\n", optarg);
                    return 1;
                }
                break;
            case 'r':
            case 'S':
                int64_t val;
                if (parse_int64(optarg, &val) != 0 || val <= 0) {
                    fprintf(stderr, "Error: Invalid positive integer for --%s: '%s'\n", 
                            (opt_char == 'r' ? "clock-rate" : "scale"), optarg);
                    return 1;
                }
                if (opt_char == 'r') config->clock_rate = (uint32_t)val;
                else config->scale_factor = (uint32_t)val;
                break;
            case '?': print_usage(argv[0]); return 1;
            default: abort();
        }
    }

    // 4. Post-parsing validation
    if (config->step_mode && config->cycles_to_run != -1) {
        fprintf(stderr, "Error: --step and --cycles options cannot be used together.\n");
        return 1;
    }
    if (optind >= argc) {
        fprintf(stderr, "Error: Missing required ROM path argument.\n");
        print_usage(argv[0]);
        return 1;
    }
    config->rom_path = argv[optind];

    // 5. Print final configuration
    printf("Emulator Configuration:\n");
    printf("-----------------------\n");
    printf("ROM Path:      %s\n", config->rom_path);
    printf("Step Mode:     %s\n", config->step_mode ? "ON" : "OFF");
    printf("Legacy Mode:   %s\n", config->legacy_mode ? "ON" : "OFF");
    if (config->cycles_to_run != -1) {
        printf("Cycles to Run: %ld\n", config->cycles_to_run);
    }
    printf("Clock Rate:    %u Hz\n", config->clock_rate); // Use %u for unsigned
    printf("Scale Factor:  %ux\n", config->scale_factor); // Use %u for unsigned
    printf("-----------------------\n");

    return 0;
}
