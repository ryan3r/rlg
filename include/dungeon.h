#ifndef DUNGEON_H
#define DUNGEON_H

// Copied from Dr. Sheaffer's solution (rlg327)
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <endian.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>

/* Very slow seed: 686846853 */

#include <heap.h>
#include <util.h>

/* Returns true if random float in [0,1] is less than *
 * numerator/denominator.  Uses only integer math.    */
# define rand_under(numerator, denominator) \
	(rand() < ((RAND_MAX / denominator) * numerator))

/* Returns random integer in [min, max]. */
# define rand_range(min, max) ((rand() % (((max) + 1) - (min))) + (min))
# define UNUSED(f) ((void) f)

typedef struct corridor_path {
	heap_node_t *hn;
	uint8_t pos[2];
	uint8_t from[2];
	int32_t cost;
} corridor_path_t;

typedef enum dim {
	dim_x,
	dim_y,
	num_dims
} dim_t;

typedef int16_t pair_t[num_dims];

#define DUNGEON_X              80
#define DUNGEON_Y              21
#define MIN_ROOMS              5
#define MAX_ROOMS              9
#define ROOM_MIN_X             4
#define ROOM_MIN_Y             2
#define ROOM_MAX_X             14
#define ROOM_MAX_Y             8

#define mappair(pair) (d->map[pair[dim_y]][pair[dim_x]])
#define mapxy(x, y) (d->map[y][x])
#define hardnesspair(pair) (d->hardness[pair[dim_y]][pair[dim_x]])
#define hardnessxy(x, y) (d->hardness[y][x])

typedef enum __attribute__ ((__packed__)) terrain_type {
	ter_debug,
	ter_wall,
	ter_wall_immutable,
	ter_floor,
	ter_floor_room,
	ter_floor_hall,
} terrain_type_t;

typedef struct room {
	pair_t position;
	pair_t size;
} room_t;

typedef struct dungeon {
	vector_t rooms;
	terrain_type_t map[DUNGEON_Y][DUNGEON_X];
	/* Since hardness is usually not used, it would be expensive to pull it *
	 * into cache every time we need a map cell, so we store it in a        *
	 * parallel array, rather than using a structure to represent the       *
	 * cells.  We may want a cell structure later, but from a performanace  *
	 * perspective, it would be a bad idea to ever have the map be part of  *
	 * that structure.  Pathfinding will require efficient use of the map,  *
	 * and pulling in unnecessary data with each map cell would add a lot   *
	 * of overhead to the memory system.                                    */
	uint8_t hardness[DUNGEON_Y][DUNGEON_X];
} dungeon_t;

int gen_dungeon(dungeon_t *d);
void render_dungeon(dungeon_t *d);
void delete_dungeon(dungeon_t *d);
void init_dungeon(dungeon_t *d);

#endif
