#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    char *load_file;
    char *save_file;
    bool chosen_position;
    uint8_t player_x;
    uint8_t player_y;
} arguments_t;

// parse the command line arguments/open files for loading/saving
void parse_args(arguments_t *arguments, int argc, char **argv);

#endif
