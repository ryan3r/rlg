#include <argp.h>
#include <arguments.h>
#include <macros.h>
#include <stdlib.h>
#include <string.h>

// program information
const char *program_name = "dungeon";
const char *bug_email = "<bugs@404.com>";

static char doc[] = "dungeon 1.02 -- A basic dungeon generator";
static char args_doc[] = "";

// the command line options
static struct argp_option options[] = {
    {"load", 'l', "FILE", OPTION_ARG_OPTIONAL, "Load a dungeon file"},
    {"save", 's', "FILE", OPTION_ARG_OPTIONAL, "Save a dungeon file"},
    {"player", 'p', "(y,x)", 0, "The player position"},
    {0}};

// the default dungeon file
char *dungeon_file_name = "dungeon";

// parse a single command line option
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    arguments_t *arguments = state->input;

    switch (key) {
        // load a file
        case 'l':
            arguments->load_file = arg ? arg : dungeon_file_name;
            break;

        // save a file
        case 's':
            arguments->save_file = arg ? arg : dungeon_file_name;
            break;
        
        // place that player
        case 'p':
            arguments->chosen_position = true;
            sscanf(arg, "(%hhu,%hhu)", &arguments->player_y, &arguments->player_x);
            break;

        // use defaults
        case ARGP_KEY_NO_ARGS:
            break;

        case ARGP_KEY_ARG:
            argp_usage(state);
            break;

        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};

// parse the command line arguments
void parse_args(arguments_t *arguments, int argc, char **argv) {
    memset(arguments, 0, sizeof(arguments_t));

    argp_parse(&argp, argc, argv, 0, 0, arguments);
}
