#include <stdio.h>
#include <stdlib.h>
#include <dungeon.h>
#include <util.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>

// the hardness of the rock in the dungeon
hardness_t hardness_matrix[DUNGEON_HEIGHT][DUNGEON_WIDTH];

// the rooms in the dungeon
vector_t room_vector;

// check if a point is in bounds
#define OUT_OF_BOUNDS(x, y) \
	(x <= 0 || y <= 0 || x >= DUNGEON_WIDTH - 1 || y >= DUNGEON_HEIGHT - 1)

// generate the harnesses for the dungeon
void generate_hardness_matrix() {
	FOR(y, DUNGEON_HEIGHT) {
		FOR(x, DUNGEON_WIDTH) {
			// set outer wall harness to 255
			if(OUT_OF_BOUNDS(x, y)) {
				hardness_matrix[y][x] = BOUNDARY;

				continue;
			}

			hardness_matrix[y][x] = rand() % 253 + 2;
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
		!OUT_OF_BOUNDS(newRoom.x - 1, newRoom.y - 1) &&
		!OUT_OF_BOUNDS(newRoom.x + newRoom.width + 1, newRoom.y + newRoom.height + 1);
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
				hardness_matrix[m][n] = ROOM;
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

// check if we are in the ending room
#define IS_END(m, n) \
	((roomB.x <= m && m <= roomB.x + roomB.width) && \
	 (roomB.y <= n && n <= roomB.y + roomB.height))

// check if we can move forward one space
#define CAN_MOVE_FORWARD(x, y) \
	(!OUT_OF_BOUNDS(x + x_move * x_direction, y + !x_move * y_direction) && \
	!OUT_OF_BOUNDS(x + x_move * x_direction * 2, y + !x_move * y_direction * 2) && \
	(\
		(hardness_matrix[y + !x_move * y_direction][x + x_move * x_direction] || \
			IS_END(x + x_move * x_direction, y + !x_move * y_direction)) && \
		 (hardness_matrix[y + !x_move * y_direction * 2][x + x_move * x_direction * 2] || \
			 IS_END(x + x_move * x_direction * 2, y + !x_move * y_direction * 2)) \
	 ))

// create a hall way between two rooms
void connect_rooms(room_t roomA, room_t roomB) {
	// get the direction we want to go in
	int x_direction = roomB.x - roomA.x;
	int y_direction = roomB.y - roomA.y;

	// choose a starting point
	int x = roomA.x,
		y = roomA.y;

	bool movedX = true;
	bool movedY = true;

	// move to the part of this room that is closest to the other room
	if(x_direction >= roomA.width) {
		x += roomA.width;
	}
	else if(x_direction > 0) {
		x += x_direction;
		x_direction = 0;
	}
	else {
		movedX = false;
	}

	if(y_direction >= roomA.height) {
		y += roomA.height;
	}
	else if(y_direction > 0) {
		y += y_direction;
		y_direction = 0;
	}
	else {
		movedY = false;
	}

	// convert the directions to +-1
	if(x_direction != 0) x_direction /= abs(x_direction);
	if(y_direction != 0) y_direction /= abs(y_direction);

	// move out of the room
	if(!movedX && !movedY) x += x_direction;

	bool x_move = true;

	// draw the line
	while(!IS_END(x, y)) {
		// place part of the path
		hardness_matrix[y][x] = CORRIDOR;

		// try moving up and down and take the fastest
		if(!CAN_MOVE_FORWARD(x, y)) {
			int up = x_move ? y : x;
			int down = x_move ? y : x;

			// go up
			while(!CAN_MOVE_FORWARD((x_move ? x : up), (x_move ? up : y)) &&
				up < (x_move ? DUNGEON_HEIGHT : DUNGEON_WIDTH))
					++up;

			// go down
			while(!CAN_MOVE_FORWARD((x_move ? x : down), (x_move ? down : y)) &&
				down >= 0)
				--down;

			// choose the shotest path
			bool goUp = up - (x_move ? x : y) > (x_move ? x : y) - down;

			// draw the path
			while((x_move ? y : x) != (goUp ? up : down)) {
				*(x_move ? &y : &x) += goUp ? 1 : -1;
				hardness_matrix[y][x] = CORRIDOR;
			}
		}

		// move to the next spot in the path
		*(x_move ? &x : &y) += x_move ? x_direction : y_direction;

		// change directions
		if(roomB.x <= x && x <= roomB.x + roomB.width) {
			x_move = false;
			y_direction = roomB.y == y ? 0 : (roomB.y - y) / abs(roomB.y - y);
		}

		if(roomB.y <= y && y <= roomB.y + roomB.height) {
			x_move = true;
			x_direction = roomB.x == x ? 0 : (roomB.x - x) / abs(roomB.x - x);
		}
	}

	// draw the final hash if we don't have one yet
	if(hardness_matrix[y][x]) {
		hardness_matrix[y][x] = CORRIDOR;
	}
}

#undef OUT_OF_BOUNDS
#undef INTERSECTS
#undef CAN_MOVE_FORWARD
#undef IS_END
