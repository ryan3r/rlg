// Copied from Jeremy's solution (rlg327)
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <endian.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>

/* Very slow seed: 686846853 */

#include <macros.h>
#include <heap.h>
#include <vector.h>
#include <dungeon.h>

static uint32_t in_room(dungeon_t *d, int16_t y, int16_t x)
{
	int i;

	for (i = 0; i < d->rooms.length; i++) {
		if ((x >= vector_get(&d->rooms, room_t, i).position[dim_x]) &&
				(x < (vector_get(&d->rooms, room_t, i).position[dim_x] + vector_get(&d->rooms, room_t, i).size[dim_x])) &&
				(y >= vector_get(&d->rooms, room_t, i).position[dim_y]) &&
				(y < (vector_get(&d->rooms, room_t, i).position[dim_y] + vector_get(&d->rooms, room_t, i).size[dim_y]))) {
			return 1;
		}
	}

	return 0;
}

static uint32_t adjacent_to_room(dungeon_t *d, int16_t y, int16_t x)
{
	return (mapxy(x - 1, y) == ter_floor_room ||
					mapxy(x + 1, y) == ter_floor_room ||
					mapxy(x, y - 1) == ter_floor_room ||
					mapxy(x, y + 1) == ter_floor_room);
}

static uint32_t is_open_space(dungeon_t *d, int16_t y, int16_t x)
{
	return !hardnessxy(x, y);
}

static int32_t corridor_path_cmp(const void *key, const void *with) {
	return ((corridor_path_t *) key)->cost - ((corridor_path_t *) with)->cost;
}

static void dijkstra_corridor(dungeon_t *d, pair_t from, pair_t to)
{
	static corridor_path_t path[DUNGEON_Y][DUNGEON_X], *p;
	static uint32_t initialized = 0;
	heap_t h;
	uint32_t x, y;

	if (!initialized) {
		for (y = 0; y < DUNGEON_Y; y++) {
			for (x = 0; x < DUNGEON_X; x++) {
				path[y][x].pos[dim_y] = y;
				path[y][x].pos[dim_x] = x;
			}
		}
		initialized = 1;
	}

	for (y = 0; y < DUNGEON_Y; y++) {
		for (x = 0; x < DUNGEON_X; x++) {
			path[y][x].cost = INT_MAX;
		}
	}

	path[from[dim_y]][from[dim_x]].cost = 0;

	heap_init(&h, corridor_path_cmp, NULL);

	for (y = 0; y < DUNGEON_Y; y++) {
		for (x = 0; x < DUNGEON_X; x++) {
			if (mapxy(x, y) != ter_wall_immutable) {
				path[y][x].hn = heap_insert(&h, &path[y][x]);
			} else {
				path[y][x].hn = NULL;
			}
		}
	}

	while ((p = heap_remove_min(&h))) {
		p->hn = NULL;

		if ((p->pos[dim_y] == to[dim_y]) && p->pos[dim_x] == to[dim_x]) {
			for (x = to[dim_x], y = to[dim_y];
					 (x != from[dim_x]) || (y != from[dim_y]);
					 p = &path[y][x], x = p->from[dim_x], y = p->from[dim_y]) {
				if (mapxy(x, y) != ter_floor_room) {
					mapxy(x, y) = ter_floor_hall;
					hardnessxy(x, y) = 0;
				}
			}
			heap_delete(&h);
			return;
		}

		if ((path[p->pos[dim_y] - 1][p->pos[dim_x]].hn) &&
				(path[p->pos[dim_y] - 1][p->pos[dim_x]].cost >
				 p->cost + hardnesspair(p->pos))) {
			path[p->pos[dim_y] - 1][p->pos[dim_x]].cost =
				p->cost + hardnesspair(p->pos);
			path[p->pos[dim_y] - 1][p->pos[dim_x]].from[dim_y] = p->pos[dim_y];
			path[p->pos[dim_y] - 1][p->pos[dim_x]].from[dim_x] = p->pos[dim_x];
			heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1][p->pos[dim_x]    ].hn);
		}
		if ((path[p->pos[dim_y]    ][p->pos[dim_x] - 1].hn) &&
				(path[p->pos[dim_y]    ][p->pos[dim_x] - 1].cost >
				 p->cost + hardnesspair(p->pos))) {
			path[p->pos[dim_y]    ][p->pos[dim_x] - 1].cost =
				p->cost + hardnesspair(p->pos);
			path[p->pos[dim_y]    ][p->pos[dim_x] - 1].from[dim_y] = p->pos[dim_y];
			path[p->pos[dim_y]    ][p->pos[dim_x] - 1].from[dim_x] = p->pos[dim_x];
			heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ][p->pos[dim_x] - 1].hn);
		}
		if ((path[p->pos[dim_y]    ][p->pos[dim_x] + 1].hn) &&
				(path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost >
				 p->cost + hardnesspair(p->pos))) {
			path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost =
				p->cost + hardnesspair(p->pos);
			path[p->pos[dim_y]    ][p->pos[dim_x] + 1].from[dim_y] = p->pos[dim_y];
			path[p->pos[dim_y]    ][p->pos[dim_x] + 1].from[dim_x] = p->pos[dim_x];
			heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ][p->pos[dim_x] + 1].hn);
		}
		if ((path[p->pos[dim_y] + 1][p->pos[dim_x]    ].hn) &&
				(path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost >
				 p->cost + hardnesspair(p->pos))) {
			path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost =
				p->cost + hardnesspair(p->pos);
			path[p->pos[dim_y] + 1][p->pos[dim_x]    ].from[dim_y] = p->pos[dim_y];
			path[p->pos[dim_y] + 1][p->pos[dim_x]    ].from[dim_x] = p->pos[dim_x];
			heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1][p->pos[dim_x]    ].hn);
		}
	}
}

/* This is a cut-and-paste of the above.  The code is modified to  *
 * calculate paths based on inverse hardnesses so that we get a    *
 * high probability of creating at least one cycle in the dungeon. */
static void dijkstra_corridor_inv(dungeon_t *d, pair_t from, pair_t to)
{
	static corridor_path_t path[DUNGEON_Y][DUNGEON_X], *p;
	static uint32_t initialized = 0;
	heap_t h;
	uint32_t x, y;

	if (!initialized) {
		for (y = 0; y < DUNGEON_Y; y++) {
			for (x = 0; x < DUNGEON_X; x++) {
				path[y][x].pos[dim_y] = y;
				path[y][x].pos[dim_x] = x;
			}
		}
		initialized = 1;
	}

	for (y = 0; y < DUNGEON_Y; y++) {
		for (x = 0; x < DUNGEON_X; x++) {
			path[y][x].cost = INT_MAX;
		}
	}

	path[from[dim_y]][from[dim_x]].cost = 0;

	heap_init(&h, corridor_path_cmp, NULL);

	for (y = 0; y < DUNGEON_Y; y++) {
		for (x = 0; x < DUNGEON_X; x++) {
			if (mapxy(x, y) != ter_wall_immutable) {
				path[y][x].hn = heap_insert(&h, &path[y][x]);
			} else {
				path[y][x].hn = NULL;
			}
		}
	}

	while ((p = heap_remove_min(&h))) {
		p->hn = NULL;

		if ((p->pos[dim_y] == to[dim_y]) && p->pos[dim_x] == to[dim_x]) {
			for (x = to[dim_x], y = to[dim_y];
					 (x != from[dim_x]) || (y != from[dim_y]);
					 p = &path[y][x], x = p->from[dim_x], y = p->from[dim_y]) {
				if (mapxy(x, y) != ter_floor_room) {
					mapxy(x, y) = ter_floor_hall;
					hardnessxy(x, y) = 0;
				}
			}
			heap_delete(&h);
			return;
		}

#define hardnesspair_inv(p) (is_open_space(d, p[dim_y], p[dim_x]) ? 127 :     \
    (adjacent_to_room(d, p[dim_y], p[dim_x]) ? 191 : \
    (255 - hardnesspair(p))))

		if ((path[p->pos[dim_y] - 1][p->pos[dim_x]    ].hn) &&
				(path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost >
				 p->cost + hardnesspair_inv(p->pos))) {
			path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost =
				p->cost + hardnesspair_inv(p->pos);
			path[p->pos[dim_y] - 1][p->pos[dim_x]    ].from[dim_y] = p->pos[dim_y];
			path[p->pos[dim_y] - 1][p->pos[dim_x]    ].from[dim_x] = p->pos[dim_x];
			heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1][p->pos[dim_x]    ].hn);
		}
		if ((path[p->pos[dim_y]    ][p->pos[dim_x] - 1].hn) &&
				(path[p->pos[dim_y]    ][p->pos[dim_x] - 1].cost >
				 p->cost + hardnesspair_inv(p->pos))) {
			path[p->pos[dim_y]    ][p->pos[dim_x] - 1].cost =
				p->cost + hardnesspair_inv(p->pos);
			path[p->pos[dim_y]    ][p->pos[dim_x] - 1].from[dim_y] = p->pos[dim_y];
			path[p->pos[dim_y]    ][p->pos[dim_x] - 1].from[dim_x] = p->pos[dim_x];
			heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ][p->pos[dim_x] - 1].hn);
		}
		if ((path[p->pos[dim_y]    ][p->pos[dim_x] + 1].hn) &&
				(path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost >
				 p->cost + hardnesspair_inv(p->pos))) {
			path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost =
				p->cost + hardnesspair_inv(p->pos);
			path[p->pos[dim_y]    ][p->pos[dim_x] + 1].from[dim_y] = p->pos[dim_y];
			path[p->pos[dim_y]    ][p->pos[dim_x] + 1].from[dim_x] = p->pos[dim_x];
			heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ][p->pos[dim_x] + 1].hn);
		}
		if ((path[p->pos[dim_y] + 1][p->pos[dim_x]    ].hn) &&
				(path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost >
				 p->cost + hardnesspair_inv(p->pos))) {
			path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost =
				p->cost + hardnesspair_inv(p->pos);
			path[p->pos[dim_y] + 1][p->pos[dim_x]    ].from[dim_y] = p->pos[dim_y];
			path[p->pos[dim_y] + 1][p->pos[dim_x]    ].from[dim_x] = p->pos[dim_x];
			heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1][p->pos[dim_x]    ].hn);
		}
	}
}

/* Chooses a random point inside each room and connects them with a *
 * corridor.  Random internal points prevent corridors from exiting *
 * rooms in predictable locations.                                  */
static int connect_two_rooms(dungeon_t *d, room_t *r1, room_t *r2)
{
	pair_t e1, e2;

	e1[dim_y] = rand_range(r1->position[dim_y], r1->position[dim_y] + r1->size[dim_y] - 1);
	e1[dim_x] = rand_range(r1->position[dim_x], r1->position[dim_x] + r1->size[dim_x] - 1);
	e2[dim_y] = rand_range(r2->position[dim_y], r2->position[dim_y] + r2->size[dim_y] - 1);
	e2[dim_x] = rand_range(r2->position[dim_x], r2->position[dim_x] + r2->size[dim_x] - 1);

	/*  return connect_two_points_recursive(d, e1, e2);*/
	dijkstra_corridor(d, e1, e2);

	return 0;
}

static int create_cycle(dungeon_t *d)
{
	/* Find the (approximately) farthest two rooms, then connect *
	 * them by the shortest path using inverted hardnesses.      */

	int32_t max, tmp, i, j, p, q;
	pair_t e1, e2;

	for (i = max = 0; i < d->rooms.length - 1; i++) {
		for (j = i + 1; j < d->rooms.length; j++) {
			tmp = (((vector_get(&d->rooms, room_t, i).position[dim_x] - vector_get(&d->rooms, room_t, j).position[dim_x])  *
                (vector_get(&d->rooms, room_t, i).position[dim_x] - vector_get(&d->rooms, room_t, j).position[dim_x])) +
                ((vector_get(&d->rooms, room_t, i).position[dim_y] - vector_get(&d->rooms, room_t, j).position[dim_y])  *
                (vector_get(&d->rooms, room_t, i).position[dim_y] - vector_get(&d->rooms, room_t, j).position[dim_y])));
			if (tmp > max) {
				max = tmp;
				p = i;
				q = j;
			}
		}
	}

	/* Can't simply call connect_two_rooms() because it doesn't *
	 * use inverse hardnesses, so duplicate it here.            */
	e1[dim_y] = rand_range(vector_get(&d->rooms, room_t, p).position[dim_y],
                (vector_get(&d->rooms, room_t, p).position[dim_y] + vector_get(&d->rooms, room_t, p).size[dim_y] - 1));
	e1[dim_x] = rand_range(vector_get(&d->rooms, room_t, p).position[dim_x],
                (vector_get(&d->rooms, room_t, p).position[dim_x] + vector_get(&d->rooms, room_t, p).size[dim_x] - 1));
	e2[dim_y] = rand_range(vector_get(&d->rooms, room_t, q).position[dim_y],
                (vector_get(&d->rooms, room_t, q).position[dim_y] + vector_get(&d->rooms, room_t, q).size[dim_y] - 1));
	e2[dim_x] = rand_range(vector_get(&d->rooms, room_t, q).position[dim_x],
                (vector_get(&d->rooms, room_t, q).position[dim_x] + vector_get(&d->rooms, room_t, q).size[dim_x] - 1));

	dijkstra_corridor_inv(d, e1, e2);

	return 0;
}

static int connect_rooms(dungeon_t *d)
{
	uint32_t i;

	for (i = 1; i < d->rooms.length; i++) {
		connect_two_rooms(d, vector_get_ptr(&d->rooms, room_t, i) - 1, vector_get_ptr(&d->rooms, room_t, i));
	}

	create_cycle(d);

	return 0;
}

int gaussian[5][5] = {
	{  1,  4,  7,  4,  1 },
	{  4, 16, 26, 16,  4 },
	{  7, 26, 41, 26,  7 },
	{  4, 16, 26, 16,  4 },
	{  1,  4,  7,  4,  1 }
};

typedef struct queue_node {
	int x, y;
	struct queue_node *next;
} queue_node_t;

static int smooth_hardness(dungeon_t *d)
{
	int32_t i, x, y;
	int32_t s, t, p, q;
	queue_node_t *head, *tail, *tmp;
    #ifdef VERBOSE_DEBUG
	FILE *out;
    #endif
	uint8_t hardness[DUNGEON_Y][DUNGEON_X];

	memset(&hardness, 0, sizeof (hardness));

	/* Seed with some values */
	for (i = 1; i < 255; i += 20) {
		do {
			x = rand() % DUNGEON_X;
			y = rand() % DUNGEON_Y;
		} while (hardness[y][x]);
		hardness[y][x] = i;
		if (i == 1) {
			head = tail = malloc(sizeof (*tail));
		} else {
			tail->next = malloc(sizeof (*tail));
			tail = tail->next;
		}
		tail->next = NULL;
		tail->x = x;
		tail->y = y;
	}

    #ifdef VERBOSE_DEBUG
	out = fopen("seeded.pgm", "w");
	fprintf(out, "P5\n%u %u\n255\n", DUNGEON_X, DUNGEON_Y);
	fwrite(&hardness, sizeof (hardness), 1, out);
	fclose(out);
    #endif

	/* Diffuse the vaules to fill the space */
	while (head) {
		x = head->x;
		y = head->y;
		i = hardness[y][x];

		if (x - 1 >= 0 && y - 1 >= 0 && !hardness[y - 1][x - 1]) {
			hardness[y - 1][x - 1] = i;
			tail->next = malloc(sizeof (*tail));
			tail = tail->next;
			tail->next = NULL;
			tail->x = x - 1;
			tail->y = y - 1;
		}
		if (x - 1 >= 0 && !hardness[y][x - 1]) {
			hardness[y][x - 1] = i;
			tail->next = malloc(sizeof (*tail));
			tail = tail->next;
			tail->next = NULL;
			tail->x = x - 1;
			tail->y = y;
		}
		if (x - 1 >= 0 && y + 1 < DUNGEON_Y && !hardness[y + 1][x - 1]) {
			hardness[y + 1][x - 1] = i;
			tail->next = malloc(sizeof (*tail));
			tail = tail->next;
			tail->next = NULL;
			tail->x = x - 1;
			tail->y = y + 1;
		}
		if (y - 1 >= 0 && !hardness[y - 1][x]) {
			hardness[y - 1][x] = i;
			tail->next = malloc(sizeof (*tail));
			tail = tail->next;
			tail->next = NULL;
			tail->x = x;
			tail->y = y - 1;
		}
		if (y + 1 < DUNGEON_Y && !hardness[y + 1][x]) {
			hardness[y + 1][x] = i;
			tail->next = malloc(sizeof (*tail));
			tail = tail->next;
			tail->next = NULL;
			tail->x = x;
			tail->y = y + 1;
		}
		if (x + 1 < DUNGEON_X && y - 1 >= 0 && !hardness[y - 1][x + 1]) {
			hardness[y - 1][x + 1] = i;
			tail->next = malloc(sizeof (*tail));
			tail = tail->next;
			tail->next = NULL;
			tail->x = x + 1;
			tail->y = y - 1;
		}
		if (x + 1 < DUNGEON_X && !hardness[y][x + 1]) {
			hardness[y][x + 1] = i;
			tail->next = malloc(sizeof (*tail));
			tail = tail->next;
			tail->next = NULL;
			tail->x = x + 1;
			tail->y = y;
		}
		if (x + 1 < DUNGEON_X && y + 1 < DUNGEON_Y && !hardness[y + 1][x + 1]) {
			hardness[y + 1][x + 1] = i;
			tail->next = malloc(sizeof (*tail));
			tail = tail->next;
			tail->next = NULL;
			tail->x = x + 1;
			tail->y = y + 1;
		}

		tmp = head;
		head = head->next;
		free(tmp);
	}

	/* And smooth it a bit with a gaussian convolution */
	for (y = 0; y < DUNGEON_Y; y++) {
		for (x = 0; x < DUNGEON_X; x++) {
			for (s = t = p = 0; p < 5; p++) {
				for (q = 0; q < 5; q++) {
					if (y + (p - 2) >= 0 && y + (p - 2) < DUNGEON_Y &&
						  x + (q - 2) >= 0 && x + (q - 2) < DUNGEON_X) {
						s += gaussian[p][q];
						t += hardness[y + (p - 2)][x + (q - 2)] * gaussian[p][q];
					}
				}
			}
			d->hardness[y][x] = t / s;
		}
	}
	/* Let's do it again, until it's smooth like Kenny G. */
	for (y = 0; y < DUNGEON_Y; y++) {
		for (x = 0; x < DUNGEON_X; x++) {
			for (s = t = p = 0; p < 5; p++) {
				for (q = 0; q < 5; q++) {
					if (y + (p - 2) >= 0 && y + (p - 2) < DUNGEON_Y &&
						  x + (q - 2) >= 0 && x + (q - 2) < DUNGEON_X) {
						s += gaussian[p][q];
						t += hardness[y + (p - 2)][x + (q - 2)] * gaussian[p][q];
					}
				}
			}
			d->hardness[y][x] = t / s;
		}
	}


    #ifdef VERBOSE_DEBUG
	out = fopen("diffused.pgm", "w");
	fprintf(out, "P5\n%u %u\n255\n", DUNGEON_X, DUNGEON_Y);
	fwrite(&hardness, sizeof (hardness), 1, out);
	fclose(out);

	out = fopen("smoothed.pgm", "w");
	fprintf(out, "P5\n%u %u\n255\n", DUNGEON_X, DUNGEON_Y);
	fwrite(&d->hardness, sizeof (d->hardness), 1, out);
	fclose(out);
    #endif

	return 0;
}

static int empty_dungeon(dungeon_t *d)
{
	uint8_t x, y;

	smooth_hardness(d);
	for (y = 0; y < DUNGEON_Y; y++) {
		for (x = 0; x < DUNGEON_X; x++) {
			mapxy(x, y) = ter_wall;
			if (y == 0 || y == DUNGEON_Y - 1 ||
					x == 0 || x == DUNGEON_X - 1) {
				mapxy(x, y) = ter_wall_immutable;
				hardnessxy(x, y) = 255;
			}
		}
	}

	return 0;
}

static int place_rooms(dungeon_t *d)
{
	pair_t p;
	uint32_t i;
	int success;
	room_t *r;

	for (success = 0; !success; ) {
		success = 1;
		for (i = 0; success && i < d->rooms.length; i++) {
			r = vector_get_ptr(&d->rooms, room_t, i);
			r->position[dim_x] = 1 + rand() % (DUNGEON_X - 2 - r->size[dim_x]);
			r->position[dim_y] = 1 + rand() % (DUNGEON_Y - 2 - r->size[dim_y]);
			for (p[dim_y] = r->position[dim_y] - 1;
					 success && p[dim_y] < r->position[dim_y] + r->size[dim_y] + 1;
					 p[dim_y]++) {
				for (p[dim_x] = r->position[dim_x] - 1;
						 success && p[dim_x] < r->position[dim_x] + r->size[dim_x] + 1;
						 p[dim_x]++) {
					if (mappair(p) >= ter_floor) {
						success = 0;
						empty_dungeon(d);
					} else if ((p[dim_y] != r->position[dim_y] - 1)              &&
                            (p[dim_y] != r->position[dim_y] + r->size[dim_y]) &&
                            (p[dim_x] != r->position[dim_x] - 1)              &&
                            (p[dim_x] != r->position[dim_x] + r->size[dim_x])) {
						mappair(p) = ter_floor_room;
						hardnesspair(p) = 0;
					}
				}
			}
		}
	}

	return 0;
}

static int make_rooms(dungeon_t *d)
{
	uint32_t i;

	for (i = MIN_ROOMS; i < MAX_ROOMS && rand_under(6, 8); i++);

	vector_init(&d->rooms, sizeof(vector_t), i);
	d->rooms.length = i;

	for (i = 0; i < d->rooms.length; i++) {
		vector_get(&d->rooms, room_t, i).size[dim_x] = ROOM_MIN_X;
		vector_get(&d->rooms, room_t, i).size[dim_y] = ROOM_MIN_Y;
		while (rand_under(3, 4) && vector_get(&d->rooms, room_t, i).size[dim_x] < ROOM_MAX_X) {
			vector_get(&d->rooms, room_t, i).size[dim_x]++;
		}
		while (rand_under(3, 4) && vector_get(&d->rooms, room_t, i).size[dim_y] < ROOM_MAX_Y) {
			vector_get(&d->rooms, room_t, i).size[dim_y]++;
		}
	}

	return 0;
}

int gen_dungeon(dungeon_t *d)
{
	empty_dungeon(d);

	do {
		make_rooms(d);
	} while (place_rooms(d));
	connect_rooms(d);

	return 0;
}

void render_dungeon(dungeon_t *d)
{
	pair_t p;

	for (p[dim_y] = 0; p[dim_y] < DUNGEON_Y; p[dim_y]++) {
		for (p[dim_x] = 0; p[dim_x] < DUNGEON_X; p[dim_x]++) {
			switch (mappair(p)) {
			case ter_wall:
			case ter_wall_immutable:
				putchar(' ');
				break;
			case ter_floor:
			case ter_floor_room:
				putchar('.');
				break;
			case ter_floor_hall:
				putchar('#');
				break;
			case ter_debug:
				putchar('*');
				fprintf(stderr, "Debug character at %d, %d\n", p[dim_y], p[dim_x]);
				break;
			}
		}
		putchar('\n');
	}
}

void delete_dungeon(dungeon_t *d)
{
	vector_destroy(&d->rooms);
}

void init_dungeon(dungeon_t *d)
{
	UNUSED(in_room);
	empty_dungeon(d);
}

// the name of the game dir
char rlg_dir_name[] = ".rlg327";

// create the .rlg327 directory and switch the working directory to it
int init_game_dir() {
	int status = 0;

	// find the home directory
	char *home_dir = getenv("HOME");

	// find absolute path to our save directory
	size_t length = strlen(home_dir) + sizeof(rlg_dir_name) + 2;
	char *path = malloc(length);
	snprintf(path, length, "%s/%s", home_dir, rlg_dir_name);

	// make the directory if it doesn't exist
	if(access(path, F_OK) == -1) {
		status = mkdir(path, 0777);
	}

	// move to the save directory
	if(!status) {
		status = chdir(path);
	}

	// print any error messages
	if(status) {
		perror("Failed to initialize save directory");
	}

	free(path);

	return status;
}

// initialize the terrain map of a newly loaded dungeon
static void init_terrain_map(dungeon_t* dungeon) {
	FOR(y, DUNGEON_Y)
		FOR(x, DUNGEON_X) {
			// room or corridor, assume corridor for now
			if(!dungeon->hardness[y][x]) {
				dungeon->map[y][x] = ter_floor_hall;
			}
			else {
				dungeon->map[y][x] = dungeon->hardness[y][x] == 255 ?
					ter_wall_immutable : ter_wall;
			}
		}

	// place the rooms in the map
	vector_for(room_t, room, &dungeon->rooms) {
		for(uint32_t y = room->position[dim_y],
				end = room->position[dim_y] + room->size[dim_y]; y < end; ++y) {
			for(uint32_t x = room->position[dim_x],
					end_x = room->position[dim_x] + room->size[dim_x]; x < end_x; ++x) {
				dungeon->map[y][x] = ter_floor_room;
			}
		}
	}
}

char rlg_file_marker[] = "RLG327-S2018";

// write the dungeon to a save file
int save_dungeon(dungeon_t *dungeon, char *file_name) {
	FILE *file = fopen(file_name, "wb");

	if(file == NULL) return -1;

	// prepare the header
	uint8_t header[20];

	memcpy(header, rlg_file_marker, sizeof(rlg_file_marker) - 1);

	uint32_t version = htobe32(0);
	memcpy(header + 12, &version, sizeof(version));

	// calculate the file size
	uint32_t file_size = htobe32(1699 + dungeon->rooms.length * 4);

	*((uint32_t*) (header + 16)) = file_size;

	// write the header
	fwrite(header, sizeof(header), 1, file);

	// write the hardness map to the file
	fwrite(dungeon->hardness, sizeof(dungeon->hardness), 1, file);

    // convert the rooms to a serializable format
	size_t rooms_size = dungeon->rooms.length * 4 * sizeof(uint8_t);
	uint8_t *rooms = malloc(rooms_size);
	uint32_t index = 0;

	vector_for(room_t, room, &dungeon->rooms) {
		rooms[index++] = room->position[dim_y];
		rooms[index++] = room->position[dim_x];
		rooms[index++] = room->size[dim_y];
		rooms[index++] = room->size[dim_x];
	}

	// write the rooms to the file
	fwrite(rooms, rooms_size, 1, file);

	free(rooms);

	fclose(file);

	return 0;
}

// load the dungeon from a save file
int load_dungeon(dungeon_t *dungeon, char *file_name) {
	FILE *file = fopen(file_name, "rb");

	if(file == NULL) return -1;

	// verify the header
	uint8_t header[20];

	fread(&header, sizeof(header), 1, file);

	// verify the marker
	if(memcmp(header, rlg_file_marker, sizeof(rlg_file_marker) - 1)) {
		fprintf(stderr, "Invalid file marker\n");
		return -1;
	}

	uint32_t version;

	memcpy(&version, header + 12, sizeof(version));
	version = be32toh(version);

	// verify the version
	if(version > 0) {
		fprintf(stderr, "Version %d is not supported\n", version);
		return -1;
	}

	// load the hardness map from the file
	fread(dungeon->hardness, sizeof(dungeon->hardness), 1, file);

	// load the rooms
	uint32_t file_size = be32toh(*((uint32_t*) (header + 16)));

	uint32_t rooms_size = file_size - 1699;
	vector_init(&dungeon->rooms, sizeof(room_t), rooms_size / 4);
	dungeon->rooms.length = rooms_size / 4;

	// deserialize the rooms
	uint8_t *rooms = malloc(rooms_size * sizeof(uint8_t));
	uint32_t index = 0;

	fread(rooms, rooms_size * sizeof(uint8_t), 1, file);

	vector_for(room_t, room, &dungeon->rooms) {
		room->position[dim_y] = rooms[index++];
		room->position[dim_x] = rooms[index++];
		room->size[dim_y] = rooms[index++];
		room->size[dim_x] = rooms[index++];
	}

	// initialize the terrain
	init_terrain_map(dungeon);

	free(rooms);

	// close the file
	fclose(file);

	return 0;
}
