#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <endian.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <sys/time.h>
#include <dungeon.h>

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

	init_dungeon(&d);
	gen_dungeon(&d);
	render_dungeon(&d);
	delete_dungeon(&d);

	return 0;
}
