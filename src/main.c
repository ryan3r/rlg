#include <dungeon.h>
#include <stdio.h>
#include <util.h>
#include <time.h>
#include <stdlib.h>

int main() {
	srand(time(NULL));

	generate_hardness_matrix(1);

	create_room();

	foreach(y, DUNGEON_HEIGHT) {
		foreach(x, DUNGEON_WIDTH) {
			printf("%3d ", hardness_matrix[y][x]);
		}

		printf("\n");
	}
}
