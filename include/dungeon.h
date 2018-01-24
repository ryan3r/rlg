#ifndef DUNGEON_HEADER_DEFINED
#define DUNGEON_HEADER_DEFINED

#include <util.h>
#include <stdbool.h>

// the size of the dungeon
#define DUNGEON_WIDTH 80
#define DUNGEON_HEIGHT 21

// the percentage of the dungeon that can be rooms [0, 1]
#define PERCENT_OF_DUNGON_ROOMS .2
#define MAX_FAILED_PLACEMENTS 5000
#define MIN_ROOM_SIZE 5
#define MAX_ROOM_SIZE 10

// the values for different spots
#define CORRIDOR 1
#define ROOM 0
#define BOUNDARY 255

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
vector_t room_vector;

// generate the harnesses for the dungeon
void generate_hardness_matrix();

// create a new room
bool create_room(int fails);

// create a hall way between two rooms
void connect_rooms(room_t roomA, room_t roomB);

#endif
