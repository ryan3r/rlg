#include <stdio.h>
#include <stdlib.h>
#include <dungeon.h>
#include <util.h>
#include <stdbool.h>

bool is_boundary(unsigned int x, unsigned int y) {
	return x <= 0 || y <= 0 || x >= DUNGEON_WIDTH - 1 || y >= DUNGEON_HEIGHT - 1;
}

void generate_hardness_matrix() {
	FOR(y, DUNGEON_HEIGHT) {
		FOR(x, DUNGEON_WIDTH) {
			// set outer wall harness to 255
			if(is_boundary(x, y)) {
				hardness_matrix[y][x] = 255;

				continue;
			}

			hardness_matrix[y][x] = rand() % 255 + 1;
		}
	}
}

void create_room() {
	unsigned int x = rand() % DUNGEON_WIDTH;
	unsigned int y = rand() % DUNGEON_HEIGHT;
	unsigned int height = rand() % 7 + 3;
	unsigned int width = rand() % 7 + 3;

	if(!is_boundary(x, y) && !is_boundary(x + width, y + height)) {
		for(int m = y; m < y + height; ++m) {
			for(int n = x; n < x + width; ++n) {
				hardness_matrix[m][n] = 0;
			}
		}
	}
	else {
		create_room();
	}
}
