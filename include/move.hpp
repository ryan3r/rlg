// Based on Jeremy's solution for 1.04
#ifndef MOVE_H
# define MOVE_H

# include <stdint.h>

#include <dims.hpp>

class dungeon_t;
class character_t;

void do_moves(dungeon_t *d);
uint32_t in_corner(dungeon_t *d, character_t *c);
uint32_t against_wall(dungeon_t *d, character_t *c);
void move_character(dungeon_t *d, character_t *c, pair_t &next);

#endif
