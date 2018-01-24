#include <dungeon.h>
#include <stdio.h>
#include <util.h>
#include <time.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	time_t seed;

	if(argc < 2) {
		seed = time(NULL);
	}
	else {
		seed = atoi(argv[1]);
	}

	srand(seed);

	printf("Seed: %ld\n", seed);

	vector_init(&room_vector, sizeof(room_t));

	generate_hardness_matrix(1);

	while(create_room(0));

	//FOR(i, room_vector.length)
		//for(int j = i - 1; j >= 0; --j)
			connect_rooms(vector_get(&room_vector, room_t, 0), vector_get(&room_vector, room_t, 1));

	FOR(y, DUNGEON_HEIGHT) {
		FOR(x, DUNGEON_WIDTH) {
			printf(hardness_matrix[y][x] == 0 ? "m" : hardness_matrix[y][x] == 255 ? "x" : " ");
		}

		printf("\n");
	}

	vector_destroy(&room_vector);
}
