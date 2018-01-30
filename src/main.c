#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>
#include <dungeon.h>
#include <string.h>

// TODO: Check memory leaks

int main(int argc, char *argv[])
{
	dungeon_t d;
	struct timeval tv;
	uint32_t seed;

	if (argc == 2) {
		seed = atoi(argv[1]);
	} else {
		gettimeofday(&tv, NULL);
		seed = (tv.tv_usec ^ (tv.tv_sec << 20)) & 0xffffffff;
	}

	printf("Using seed: %u\n", seed);
	srand(seed);

	if(init_game_dir()) return 1;

	init_dungeon(&d);

	if(load_dungeon(&d, "foo")) return 25;

	// gen_dungeon(&d);
	render_dungeon(&d);

	save_dungeon(&d, "foo");

	delete_dungeon(&d);

	return 0;
}
