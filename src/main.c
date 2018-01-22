#include <dungeon.h>
#include <stdio.h>
#include <util.h>
#include <time.h>
#include <stdlib.h>

int main() {
	srand(time(NULL));

	vector_init(&room_vector, sizeof(room_t));

	generate_hardness_matrix(1);

	while(create_room(0));

	FOR(y, DUNGEON_HEIGHT) {
		FOR(x, DUNGEON_WIDTH) {
			printf(hardness_matrix[y][x] == 0 ? "." : hardness_matrix[y][x] == 255 ? "x" : " ");
		}

		printf("\n");
	}

	vector_destroy(&room_vector);
}
