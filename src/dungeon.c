#include <stdio.h>
#include <stdlib.h>
#include <dungeon.h>
#include <util.h>
#include <stdbool.h>

// check if a point is in bounds
#define INBOUNDS(x, y) \
	(x <= 0 || y <= 0 || x >= DUNGEON_WIDTH - 1 || y >= DUNGEON_HEIGHT - 1)

// generate the harnesses for the dungeon
void generate_hardness_matrix() {
	FOR(y, DUNGEON_HEIGHT) {
		FOR(x, DUNGEON_WIDTH) {
			// set outer wall harness to 255
			if(INBOUNDS(x, y)) {
				hardness_matrix[y][x] = 255;

				continue;
			}

			hardness_matrix[y][x] = rand() % 254 + 1;
		}
	}
}

// check if two rooms intersect
#define INTERSECTS(a, aWidth, b, bWidth)              \
	((a <= b && b <= a + aWidth) ||                   \
	(a <= b + bWidth && b + bWidth <= a + aWidth) ||  \
	(b <= a && a <= b + bWidth) ||                    \
	(b <= a + aWidth && a + aWidth <= b + bWidth))

// check if it intersects with other rooms or walls
bool is_valid_room(room_t newRoom) {
	bool isOk = true;

	// check other rooms
	vector_for(room_t, room, &room_vector) {
		if(INTERSECTS(room->x, room->width, newRoom.x, newRoom.width) &&
			INTERSECTS(room->y, room->height, newRoom.y, newRoom.height)) {
				isOk = false;
				break;
		}
	}

	return isOk &&
		// check walls
		!INBOUNDS(newRoom.x - 1, newRoom.y - 1) &&
		!INBOUNDS(newRoom.x + newRoom.width + 1, newRoom.y + newRoom.height + 1);
}

int __total_room_area = 0;

// create a new room
bool create_room(int fails) {
	// too many failed attempts
	if(fails > MAX_FAILED_PLACEMENTS ||
		__total_room_area > (DUNGEON_WIDTH * DUNGEON_HEIGHT) * PERCENT_OF_DUNGON_ROOMS) return false;

	room_t newRoom;

	// initialize the new room
	newRoom.x = rand() % DUNGEON_WIDTH;
	newRoom.y = rand() % DUNGEON_HEIGHT;
	newRoom.height = rand() % (MAX_ROOM_SIZE - MIN_ROOM_SIZE + 1) + MIN_ROOM_SIZE;
	newRoom.width = rand() % (MAX_ROOM_SIZE - MIN_ROOM_SIZE + 1) + MIN_ROOM_SIZE;

	// check if it is in bounds
	if(is_valid_room(newRoom)) {
		// add the room to the harness matrix
		for(int m = newRoom.y; m < newRoom.y + newRoom.height; ++m) {
			for(int n = newRoom.x; n < newRoom.x + newRoom.width; ++n) {
				hardness_matrix[m][n] = 0;
			}
		}

		// add this to the list of rooms
		vector_add(&room_vector, &newRoom);

		// track the area of the dungeon that is taken up by the rooms
		__total_room_area += newRoom.width * newRoom.height;

		return true;
	}
	// try again
	else {
		return create_room(fails + 1);
	}
}

#undef INTERSECTS
#undef INBOUNDS

// create a hall way between two rooms
void connect_rooms(room_t *roomA, room_t *roomB) {

}
