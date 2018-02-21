#include <arguments.h>
#include <dungeon.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <character.h>
#include <monster.h>

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
        d.player.x = arguments.player_x;
        d.player.y = arguments.player_y;

        d.entity_map[d.player.y][d.player.x] = &d.player;
    }
    // randomly place a player
    else {
        place_entity(&d, &d.player);
    }

    calc_travel_costs(&d);

    place_monsters(&d, arguments.num_monsters);

    entity_t *entity;
    
    while(entity_get_attr(&d.player, attr_alive)) {
        entity = heap_remove_min(&d.turn_queue);

        if(entity_get_attr(entity, attr_pc)) {
            // take user input
        }
        else {
            entity_move(&d, entity);
        }

        entity->turn += 1000 / entity->speed;

        if(entity_get_attr(entity, attr_alive)) {
            heap_insert(&d.turn_queue, entity);
        }

        render_dungeon(&d);

        usleep(100000);
    }

    printf(entity_get_attr(&d.player, attr_alive) ? "You won\n" : "You died\n");

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
