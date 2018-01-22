#include <stdio.h>
#include <stdlib.h>
#include <dungeon.h>
#include <util.h>
#include <stdbool.h>

// check if a point is in bounds
#define INBOUNDS(x, y) \
	(x <= 0 || y <= 0 || x >= DUNGEON_WIDTH - 1 || y >= DUNGEON_HEIGHT - 1)

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
#define INTERSECTS(a, aWidth, b, bWidth) \
	((a <= b && b <= a + aWidth) || \
	(a <= b + bWidth && b + bWidth <= a + aWidth) || \
	(b <= a && a <= b + bWidth) || \
	(b <= a + aWidth && a + aWidth <= b + bWidth))

bool create_room(int fails) {
	// too many failed attempts
	if(fails > 500) return false;

	room_t newRoom;

	// initialize the new room
	newRoom.x = rand() % DUNGEON_WIDTH;
	newRoom.y = rand() % DUNGEON_HEIGHT;
	newRoom.height = rand() % 7 + 3;
	newRoom.width = rand() % 7 + 3;

	// check if it intersects with other rooms
	bool isOk = true;

	vector_for(room_t, room, &room_vector) {
		if(INTERSECTS(room->x, room->width, newRoom.x, newRoom.width) &&
			INTERSECTS(room->y, room->height, newRoom.y, newRoom.height)) {
				isOk = false;
				break;
		}
	}

	// check if it is in bounds
	if(isOk && !INBOUNDS(newRoom.x - 1, newRoom.y - 1) &&
		!INBOUNDS(newRoom.x + newRoom.width + 1, newRoom.y + newRoom.height + 1)) {

		// add the room to the harness matrix
		for(int m = newRoom.y; m < newRoom.y + newRoom.height; ++m) {
			for(int n = newRoom.x; n < newRoom.x + newRoom.width; ++n) {
				hardness_matrix[m][n] = 0;
			}
		}

		// add this to the list of rooms
		vector_add(&room_vector, &newRoom);

		return true;
	}
	// try again
	else {
		return create_room(fails + 1);
	}
}

#undef INTERSECTS
#undef INBOUNDS
