#ifndef ARGUMENTS_H
#define ARGUMENTS_H

typedef struct {
    char *load_file;
    char *save_file;
} arguments_t;

// parse the command line arguments/open files for loading/saving
void parse_args(arguments_t *arguments, int argc, char **argv);

#endif
