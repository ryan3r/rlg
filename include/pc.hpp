// Based on Jeremy's solution
#ifndef PC_H
# define PC_H

# include <stdint.h>

#include <dims.hpp>

typedef struct dungeon dungeon_t;

typedef struct pc {
    // MSVC requires at least one field
    #ifdef _WIN32
    char __empty_field;
    #endif
} pc_t;

void pc_delete(pc_t *pc);
uint32_t pc_is_alive(dungeon_t *d);
void config_pc(dungeon_t *d);
uint32_t pc_next_pos(dungeon_t *d, pair_t &dir);
void place_pc(dungeon_t *d);
uint32_t pc_in_room(dungeon_t *d, uint32_t room);

#endif
