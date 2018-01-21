#ifndef DUNGEON_HEADER_DEFINED
#define DUNGEON_HEADER_DEFINED

#include <stdlib.h>

// the size of the dungeon
#define DUNGEON_WIDTH 15
#define DUNGEON_HEIGHT 15

// the location and size of a room
typedef struct {
	int x;
	int y;
	int width;
	int height;
} room_t;

// the harness of a section of the dungeon
typedef unsigned char hardness_t;

// the hardness of the rock in the dungeon
hardness_t hardness_matrix[DUNGEON_HEIGHT][DUNGEON_WIDTH];

// the rooms in the dungeon
//room_t *room_vector = NULL;

// generate the harnesses for the dungeon
void generate_hardness_matrix();

// create a new room
void create_room();

#endif
