#include <dungeon.h>
#include <util.h>
#include <stdio.h>
#include <test_util.h>

bool is_valid_room(room_t newRoom);

// build a room
room_t build_room(int x, int y, int width, int height) {
	room_t room;

	room.x = x;
	room.y = y;
	room.width = width;
	room.height = height;

	return room;
}

int main() {
	bool test_failed = false;

	vector_init(&room_vector, sizeof(room_t));

	// put in a room to conflict with
	room_t room = build_room(5, 2, 5, 10);

	vector_add(&room_vector, &room);

	// try to add rooms
	ASSERT(!is_valid_room(build_room(5, 5, 2, 2)), "Room in room");
	ASSERT(!is_valid_room(build_room(5, 2, 2, 2)), "Top left corrner intersect");
	ASSERT(!is_valid_room(build_room(8, 2, 2, 2)), "Top right corrner intersect");
	ASSERT(!is_valid_room(build_room(5, 10, 2, 2)), "Bottom left corrner intersect");
	ASSERT(!is_valid_room(build_room(8, 10, 2, 2)), "Top left corrner intersect");
	ASSERT(!is_valid_room(build_room(2, 5, 10, 5)), "Cross");
	ASSERT(is_valid_room(build_room(2, 2, 1, 1)), "Left of");
	ASSERT(is_valid_room(build_room(12, 2, 2, 2)), "Right of");
	ASSERT(is_valid_room(build_room(2, 14, 2, 2)), "Below");
	ASSERT(is_valid_room(build_room(2, 14, 12, 2)), "Away");

	vector_destroy(&room_vector);

	if(test_failed) return 1;

	// if we reached here all tests passed
	printf("All tests passed\n");

	return 0;
}
