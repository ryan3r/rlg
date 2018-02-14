#include <arguments.h>
#include <dungeon.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    // seed the random number generator
    struct timeval tv;
    gettimeofday(&tv, NULL);

    srand((tv.tv_usec ^ (tv.tv_sec << 20)) & 0xffffffff);

    // set the cwd and create the data dir
    if (init_game_dir()) return 1;

    // parse the command line arguments
    arguments_t arguments;
    parse_args(&arguments, argc, argv);

    // setup the dungeon
    dungeon_t d;
    init_dungeon(&d);

    // try to load a dungeon file
    if (arguments.load_file != NULL) {
        if (load_dungeon(&d, arguments.load_file)) return 1;
    }
    // generate a new dungeon
    else {
        gen_dungeon(&d);
    }

    // use a specific location
    if (arguments.chosen_position) {
        d.player[dim_x] = arguments.player_x;
        d.player[dim_y] = arguments.player_y;
    }
    // randomly place a player
    else {
        place_player(&d);
    }

    calc_travel_costs(&d);

    // display the dungeon
    render_dungeon(&d);

    for (uint32_t y = 0; y < DUNGEON_Y; y++) {
        for (uint32_t x = 0; x < DUNGEON_X; x++) {
            uint32_t cost = d.paths[y][x].cost;

            if (d.player[dim_x] == x && d.player[dim_y] == y) {
                printf("@");
            } else if (d.hardness[y][x] != 0) {
                printf(" ");
            } else {
                printf("%u", cost % 10);
            }
        }
        printf("\n");
    }

    for (uint32_t y = 0; y < DUNGEON_Y; y++) {
        for (uint32_t x = 0; x < DUNGEON_X; x++) {
            uint32_t cost = d.paths_tunneling[y][x].cost;

            if (d.player[dim_x] == x && d.player[dim_y] == y) {
                printf("@");
            } else if (d.hardness[y][x] == 255) {
                printf(" ");
            } else {
                printf("%u", cost % 10);
            }
        }
        printf("\n");
    }

    // try to save the dungeon
    if (arguments.save_file != NULL) {
        if (save_dungeon(&d, arguments.save_file)) return 1;
    }

    delete_dungeon(&d);

    if (getenv("PAUSE_ON_EXIT")) {
        printf("Execution finished.\nPress enter to close...");
        getchar();
    }

    return 0;
}
