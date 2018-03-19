// Based on Jeremy's solution
#include <stdio.h>
#include <stdint.h>
#ifdef __linux__
#include <endian.h>
#include <sys/stat.h>
#include <sys/time.h>
#else
#include <windows.h>
#endif
#include <limits.h>
#include <errno.h>

#include <dungeon.hpp>
#include <utils.hpp>
#include<heap.h>
#include <event.hpp>

#define DUMP_HARDNESS_IMAGES 0

typedef struct corridor_path {
  heap_node_t *hn;
  pair_t pos;
  pair_t from;
  int32_t cost;
} corridor_path_t;

/* Will need this later.
static uint32_t in_room(dungeon_t *d, int16_t y, int16_t x)
{
  int i;

  for (i = 0; i < d->num_rooms; i++) {
    if ((x >= d->rooms[i].position.x) &&
        (x < (d->rooms[i].position.x + d->rooms[i].size.x)) &&
        (y >= d->rooms[i].position.y) &&
        (y < (d->rooms[i].position.y + d->rooms[i].size.y))) {
      return 1;
    }
  }

  return 0;
}
*/

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

static void dijkstra_corridor(dungeon_t *d, pair_t &from, pair_t &to)
{
  static corridor_path_t path[DUNGEON_Y][DUNGEON_X], *p;
  static uint32_t initialized = 0;
  heap_t h;
  uint32_t x, y;

  if (!initialized) {
    for (y = 0; y < DUNGEON_Y; y++) {
      for (x = 0; x < DUNGEON_X; x++) {
        path[y][x].pos.y = y;
        path[y][x].pos.x = x;
      }
    }
    initialized = 1;
  }

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      path[y][x].cost = INT_MAX;
    }
  }

  path[from.y][from.x].cost = 0;

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

  while ((p = (corridor_path_t*) heap_remove_min(&h))) {
    p->hn = NULL;

    if ((p->pos.y == to.y) && p->pos.x == to.x) {
      for (x = to.x, y = to.y;
           (x != from.x) || (y != from.y);
           p = &path[y][x], x = p->from.x, y = p->from.y) {
        if (mapxy(x, y) != ter_floor_room) {
          mapxy(x, y) = ter_floor_hall;
          hardnessxy(x, y) = 0;
        }
      }
      heap_delete(&h);
      return;
    }

    if ((path[p->pos.y - 1][p->pos.x    ].hn) &&
        (path[p->pos.y - 1][p->pos.x    ].cost >
         p->cost + hardnesspair(p->pos))) {
      path[p->pos.y - 1][p->pos.x    ].cost =
        p->cost + hardnesspair(p->pos);
      path[p->pos.y - 1][p->pos.x    ].from.y = p->pos.y;
      path[p->pos.y - 1][p->pos.x    ].from.x = p->pos.x;
      heap_decrease_key_no_replace(&h, path[p->pos.y - 1]
                                           [p->pos.x    ].hn);
    }
    if ((path[p->pos.y    ][p->pos.x - 1].hn) &&
        (path[p->pos.y    ][p->pos.x - 1].cost >
         p->cost + hardnesspair(p->pos))) {
      path[p->pos.y    ][p->pos.x - 1].cost =
        p->cost + hardnesspair(p->pos);
      path[p->pos.y    ][p->pos.x - 1].from.y = p->pos.y;
      path[p->pos.y    ][p->pos.x - 1].from.x = p->pos.x;
      heap_decrease_key_no_replace(&h, path[p->pos.y    ]
                                           [p->pos.x - 1].hn);
    }
    if ((path[p->pos.y    ][p->pos.x + 1].hn) &&
        (path[p->pos.y    ][p->pos.x + 1].cost >
         p->cost + hardnesspair(p->pos))) {
      path[p->pos.y    ][p->pos.x + 1].cost =
        p->cost + hardnesspair(p->pos);
      path[p->pos.y    ][p->pos.x + 1].from.y = p->pos.y;
      path[p->pos.y    ][p->pos.x + 1].from.x = p->pos.x;
      heap_decrease_key_no_replace(&h, path[p->pos.y    ]
                                           [p->pos.x + 1].hn);
    }
    if ((path[p->pos.y + 1][p->pos.x    ].hn) &&
        (path[p->pos.y + 1][p->pos.x    ].cost >
         p->cost + hardnesspair(p->pos))) {
      path[p->pos.y + 1][p->pos.x    ].cost =
        p->cost + hardnesspair(p->pos);
      path[p->pos.y + 1][p->pos.x    ].from.y = p->pos.y;
      path[p->pos.y + 1][p->pos.x    ].from.x = p->pos.x;
      heap_decrease_key_no_replace(&h, path[p->pos.y + 1]
                                           [p->pos.x    ].hn);
    }
  }
}

/* This is a cut-and-paste of the above.  The code is modified to  *
 * calculate paths based on inverse hardnesses so that we get a    *
 * high probability of creating at least one cycle in the dungeon. */
static void dijkstra_corridor_inv(dungeon_t *d, pair_t &from, pair_t &to)
{
  static corridor_path_t path[DUNGEON_Y][DUNGEON_X], *p;
  static uint32_t initialized = 0;
  heap_t h;
  uint32_t x, y;

  if (!initialized) {
    for (y = 0; y < DUNGEON_Y; y++) {
      for (x = 0; x < DUNGEON_X; x++) {
        path[y][x].pos.y = y;
        path[y][x].pos.x = x;
      }
    }
    initialized = 1;
  }

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      path[y][x].cost = INT_MAX;
    }
  }

  path[from.y][from.x].cost = 0;

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

  while ((p = (corridor_path_t*) heap_remove_min(&h))) {
    p->hn = NULL;

    if ((p->pos.y == to.y) && p->pos.x == to.x) {
      for (x = to.x, y = to.y;
           (x != from.x) || (y != from.y);
           p = &path[y][x], x = p->from.x, y = p->from.y) {
        if (mapxy(x, y) != ter_floor_room) {
          mapxy(x, y) = ter_floor_hall;
          hardnessxy(x, y) = 0;
        }
      }
      heap_delete(&h);
      return;
    }

#define hardnesspair_inv(p) (is_open_space(d, p.y, p.x) ? 127 :     \
                             (adjacent_to_room(d, p.y, p.x) ? 191 : \
                              (255 - hardnesspair(p))))

    if ((path[p->pos.y - 1][p->pos.x    ].hn) &&
        (path[p->pos.y - 1][p->pos.x    ].cost >
         p->cost + hardnesspair_inv(p->pos))) {
      path[p->pos.y - 1][p->pos.x    ].cost =
        p->cost + hardnesspair_inv(p->pos);
      path[p->pos.y - 1][p->pos.x    ].from.y = p->pos.y;
      path[p->pos.y - 1][p->pos.x    ].from.x = p->pos.x;
      heap_decrease_key_no_replace(&h, path[p->pos.y - 1]
                                           [p->pos.x    ].hn);
    }
    if ((path[p->pos.y    ][p->pos.x - 1].hn) &&
        (path[p->pos.y    ][p->pos.x - 1].cost >
         p->cost + hardnesspair_inv(p->pos))) {
      path[p->pos.y    ][p->pos.x - 1].cost =
        p->cost + hardnesspair_inv(p->pos);
      path[p->pos.y    ][p->pos.x - 1].from.y = p->pos.y;
      path[p->pos.y    ][p->pos.x - 1].from.x = p->pos.x;
      heap_decrease_key_no_replace(&h, path[p->pos.y    ]
                                           [p->pos.x - 1].hn);
    }
    if ((path[p->pos.y    ][p->pos.x + 1].hn) &&
        (path[p->pos.y    ][p->pos.x + 1].cost >
         p->cost + hardnesspair_inv(p->pos))) {
      path[p->pos.y    ][p->pos.x + 1].cost =
        p->cost + hardnesspair_inv(p->pos);
      path[p->pos.y    ][p->pos.x + 1].from.y = p->pos.y;
      path[p->pos.y    ][p->pos.x + 1].from.x = p->pos.x;
      heap_decrease_key_no_replace(&h, path[p->pos.y    ]
                                           [p->pos.x + 1].hn);
    }
    if ((path[p->pos.y + 1][p->pos.x    ].hn) &&
        (path[p->pos.y + 1][p->pos.x    ].cost >
         p->cost + hardnesspair_inv(p->pos))) {
      path[p->pos.y + 1][p->pos.x    ].cost =
        p->cost + hardnesspair_inv(p->pos);
      path[p->pos.y + 1][p->pos.x    ].from.y = p->pos.y;
      path[p->pos.y + 1][p->pos.x    ].from.x = p->pos.x;
      heap_decrease_key_no_replace(&h, path[p->pos.y + 1]
                                           [p->pos.x    ].hn);
    }
  }
}

/* Chooses a random point inside each room and connects them with a *
 * corridor.  Random internal points prevent corridors from exiting *
 * rooms in predictable locations.                                  */
static int connect_two_rooms(dungeon_t *d, room_t *r1, room_t *r2)
{
  pair_t e1, e2;

  e1.y = rand_range(r1->position.y,
                         r1->position.y + r1->size.y - 1);
  e1.x = rand_range(r1->position.x,
                         r1->position.x + r1->size.x - 1);
  e2.y = rand_range(r2->position.y,
                         r2->position.y + r2->size.y - 1);
  e2.x = rand_range(r2->position.x,
                         r2->position.x + r2->size.x - 1);

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

  for (i = max = 0; i < d->num_rooms - 1; i++) {
    for (j = i + 1; j < d->num_rooms; j++) {
      tmp = (((d->rooms[i].position.x - d->rooms[j].position.x)  *
              (d->rooms[i].position.x - d->rooms[j].position.x)) +
             ((d->rooms[i].position.y - d->rooms[j].position.y)  *
              (d->rooms[i].position.y - d->rooms[j].position.y)));
      if (tmp > max) {
        max = tmp;
        p = i;
        q = j;
      }
    }
  }

  /* Can't simply call connect_two_rooms() because it doesn't *
   * use inverse hardnesses, so duplicate it here.            */
  e1.y = rand_range(d->rooms[p].position.y,
                         (d->rooms[p].position.y +
                          d->rooms[p].size.y - 1));
  e1.x = rand_range(d->rooms[p].position.x,
                         (d->rooms[p].position.x +
                          d->rooms[p].size.x - 1));
  e2.y = rand_range(d->rooms[q].position.y,
                         (d->rooms[q].position.y +
                          d->rooms[q].size.y - 1));
  e2.x = rand_range(d->rooms[q].position.x,
                         (d->rooms[q].position.x +
                          d->rooms[q].size.x - 1));

  dijkstra_corridor_inv(d, e1, e2);

  return 0;
}

static int connect_rooms(dungeon_t *d)
{
  uint32_t i;

  for (i = 1; i < d->num_rooms; i++) {
    connect_two_rooms(d, d->rooms + i - 1, d->rooms + i);
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
#if DUMP_HARDNESS_IMAGES
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
      head = tail = (queue_node_t*) malloc(sizeof (*tail));
    } else {
      tail->next = (queue_node_t*) malloc(sizeof (*tail));
      tail = tail->next;
    }
    tail->next = NULL;
    tail->x = x;
    tail->y = y;
  }

#if DUMP_HARDNESS_IMAGES
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
      tail->next = (queue_node_t*)  malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x - 1;
      tail->y = y - 1;
    }
    if (x - 1 >= 0 && !hardness[y][x - 1]) {
      hardness[y][x - 1] = i;
      tail->next = (queue_node_t*) malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x - 1;
      tail->y = y;
    }
    if (x - 1 >= 0 && y + 1 < DUNGEON_Y && !hardness[y + 1][x - 1]) {
      hardness[y + 1][x - 1] = i;
      tail->next = (queue_node_t*) malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x - 1;
      tail->y = y + 1;
    }
    if (y - 1 >= 0 && !hardness[y - 1][x]) {
      hardness[y - 1][x] = i;
      tail->next = (queue_node_t*) malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x;
      tail->y = y - 1;
    }
    if (y + 1 < DUNGEON_Y && !hardness[y + 1][x]) {
      hardness[y + 1][x] = i;
      tail->next = (queue_node_t*) malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x;
      tail->y = y + 1;
    }
    if (x + 1 < DUNGEON_X && y - 1 >= 0 && !hardness[y - 1][x + 1]) {
      hardness[y - 1][x + 1] = i;
      tail->next = (queue_node_t*) malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x + 1;
      tail->y = y - 1;
    }
    if (x + 1 < DUNGEON_X && !hardness[y][x + 1]) {
      hardness[y][x + 1] = i;
      tail->next = (queue_node_t*) malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x + 1;
      tail->y = y;
    }
    if (x + 1 < DUNGEON_X && y + 1 < DUNGEON_Y && !hardness[y + 1][x + 1]) {
      hardness[y + 1][x + 1] = i;
      tail->next = (queue_node_t*) malloc(sizeof (*tail));
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

#if DUMP_HARDNESS_IMAGES
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

  d->is_new = 1;

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
    for (i = 0; success && i < d->num_rooms; i++) {
      r = d->rooms + i;
      r->position.x = 1 + rand() % (DUNGEON_X - 2 - r->size.x);
      r->position.y = 1 + rand() % (DUNGEON_Y - 2 - r->size.y);
      for (p.y = r->position.y - 1;
           success && p.y < r->position.y + r->size.y + 1;
           p.y++) {
        for (p.x = r->position.x - 1;
             success && p.x < r->position.x + r->size.x + 1;
             p.x++) {
          if (mappair(p) >= ter_floor) {
            success = 0;
            empty_dungeon(d);
          } else if ((p.y != r->position.y - 1)              &&
                     (p.y != r->position.y + r->size.y) &&
                     (p.x != r->position.x - 1)              &&
                     (p.x != r->position.x + r->size.x)) {
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

  for (i = MIN_ROOMS; i < MAX_ROOMS && rand_under(6, 8); i++)
    ;
  d->num_rooms = i;
  d->rooms = (room_t*) malloc(sizeof (*d->rooms) * d->num_rooms);

  for (i = 0; i < d->num_rooms; i++) {
    d->rooms[i].size.x = ROOM_MIN_X;
    d->rooms[i].size.y = ROOM_MIN_Y;
    while (rand_under(3, 4) && d->rooms[i].size.x < ROOM_MAX_X) {
      d->rooms[i].size.x++;
    }
    while (rand_under(3, 4) && d->rooms[i].size.y < ROOM_MAX_Y) {
      d->rooms[i].size.y++;
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

void render_dungeon(dungeon_t *d){
  erase();

  pair_t p;

  for (p.y = 0; p.y < DUNGEON_Y; p.y++) {
    for (p.x = 0; p.x < DUNGEON_X; p.x++) {
      if (charpair(p)) {
        int color = (charpair(p)->symbol != '@') + 1;

        attron(COLOR_PAIR(color));
        mvaddch(p.y + 1, p.x, charpair(p)->symbol);
        attroff(COLOR_PAIR(color));
      } else {
        switch (mappair(p)) {
        case ter_staircase_down:
          attron(COLOR_PAIR(3));
          mvaddch(p.y + 1, p.x, '>');
          attroff(COLOR_PAIR(3));
          break;
        case ter_staircase_up:
          attron(COLOR_PAIR(3));
          mvaddch(p.y + 1, p.x, '<');
          attroff(COLOR_PAIR(3));
          break;
        case ter_wall:
        case ter_wall_immutable:
          mvaddch(p.y + 1, p.x, ' ');
          break;
        case ter_floor:
        case ter_floor_room:
          mvaddch(p.y + 1, p.x, '.');
          break;
        case ter_floor_hall:
          mvaddch(p.y + 1, p.x, '#');
          break;
        case ter_debug:
          mvaddch(p.y + 1, p.x, '*');
          mprintf("Debug character at %d, %d\n", p.y, p.x);
          break;
        }
      }
    }
  }
}

void delete_dungeon(dungeon_t *d)
{
  free(d->rooms);
  heap_delete(&d->events);
  memset(d->character, 0, sizeof (d->character));
}

void init_dungeon(dungeon_t *d)
{
  empty_dungeon(d);
  memset(&d->events, 0, sizeof (d->events));
  heap_init(&d->events, compare_events, event_delete);
}

int write_dungeon_map(dungeon_t *d, FILE *f)
{
  uint32_t x, y;

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      fwrite(&d->hardness[y][x], sizeof (unsigned char), 1, f);
    }
  }

  return 0;
}

int write_rooms(dungeon_t *d, FILE *f)
{
  uint32_t i;
  uint8_t p;

  for (i = 0; i < d->num_rooms; i++) {
    /* write order is xpos, ypos, width, height */
    p = d->rooms[i].position.y;
    fwrite(&p, 1, 1, f);
    p = d->rooms[i].position.x;
    fwrite(&p, 1, 1, f);
    p = d->rooms[i].size.y;
    fwrite(&p, 1, 1, f);
    p = d->rooms[i].size.x;
    fwrite(&p, 1, 1, f);
  }

  return 0;
}

uint32_t calculate_dungeon_size(dungeon_t *d)
{
  return (20 /* The semantic, version, and size */     +
          (DUNGEON_X * DUNGEON_Y) /* The hardnesses */ +
          (d->num_rooms * 4) /* Four bytes per room */);
}

#ifdef _WIN32
int htobe32(int x) { return x; }
int be32toh(int x) { return x; }
#endif

int write_dungeon(dungeon_t *d, char *file)
{
  const char *home;
  char *filename;
  FILE *f;
  size_t len;
  uint32_t be32;

  if (!file) {
    #ifdef __linux__
    if (!(home = getenv("HOME"))) {
      fprintf(stderr, "\"HOME\" is undefined.  Using working directory.\n");
      home = ".";
    }
    #else
    // get the size of the localappdata path
    DWORD env_size = GetEnvironmentVariable("LOCALAPPDATA", NULL, 0);

    // no app data
    if(!env_size) {
      fprintf(stderr, "\"LOCALAPPDATA\" is undefined.  Using working directory.\n");
      home = strdup(".");
    }
    // get the variable
    else {
      home = (char*) malloc(env_size * sizeof(char));

      GetEnvironmentVariable("LOCALAPPDATA", home, env_size * sizeof(char));
    }
    #endif

    len = (strlen(home) + strlen(SAVE_DIR) + strlen(DUNGEON_SAVE_FILE) +
           1 /* The NULL terminator */                                 +
           2 /* The slashes */);

    filename = (char*) malloc(len * sizeof (*filename));
    sprintf(filename, "%s/%s/", home, SAVE_DIR);
    makedirectory(filename);
    strcat(filename, DUNGEON_SAVE_FILE);

    if (!(f = fopen(filename, "wb"))) {
      perror(filename);
      free(filename);

      return 1;
    }
    free(filename);

    #ifndef __linux__
    free(home);
    #endif
  } else {
    if (!(f = fopen(file, "wb"))) {
      perror(file);
      exit(-1);
    }
  }

  /* The semantic, which is 6 bytes, 0-5 */
  fwrite(DUNGEON_SAVE_SEMANTIC, 1, sizeof (DUNGEON_SAVE_SEMANTIC) - 1, f);

  /* The version, 4 bytes, 6-9 */
  be32 = htobe32(DUNGEON_SAVE_VERSION);
  fwrite(&be32, sizeof (be32), 1, f);

  /* The size of the file, 4 bytes, 10-13 */
  be32 = htobe32(calculate_dungeon_size(d));
  fwrite(&be32, sizeof (be32), 1, f);

  /* The dungeon map, 1680 bytes, 14-1693 */
  write_dungeon_map(d, f);

  /* And the rooms, num_rooms * 4 bytes, 1694-end */
  write_rooms(d, f);

  fclose(f);

  return 0;
}

int read_dungeon_map(dungeon_t *d, FILE *f)
{
  uint32_t x, y;

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      fread(&d->hardness[y][x], sizeof (d->hardness[y][x]), 1, f);
      if (d->hardness[y][x] == 0) {
        /* Mark it as a corridor.  We can't recognize room cells until *
         * after we've read the room array, which we haven't done yet. */
        d->map[y][x] = ter_floor_hall;
      } else if (d->hardness[y][x] == 255) {
        d->map[y][x] = ter_wall_immutable;
      } else {
        d->map[y][x] = ter_wall;
      }
    }
  }


  return 0;
}

int read_rooms(dungeon_t *d, FILE *f)
{
  uint32_t i;
  uint32_t x, y;
  uint8_t p;

  for (i = 0; i < d->num_rooms; i++) {
    fread(&p, 1, 1, f);
    d->rooms[i].position.y = p;
    fread(&p, 1, 1, f);
    d->rooms[i].position.x = p;
    fread(&p, 1, 1, f);
    d->rooms[i].size.y = p;
    fread(&p, 1, 1, f);
    d->rooms[i].size.x = p;

    if (d->rooms[i].size.x < 1             ||
        d->rooms[i].size.y < 1             ||
        d->rooms[i].size.x > DUNGEON_X - 1 ||
        d->rooms[i].size.y > DUNGEON_X - 1) {
      fprintf(stderr, "Invalid room size in restored dungeon.\n");

      exit(-1);
    }

    if (d->rooms[i].position.x < 1                                       ||
        d->rooms[i].position.y < 1                                       ||
        d->rooms[i].position.x > DUNGEON_X - 1                           ||
        d->rooms[i].position.y > DUNGEON_Y - 1                           ||
        d->rooms[i].position.x + d->rooms[i].size.x > DUNGEON_X - 1 ||
        d->rooms[i].position.x + d->rooms[i].size.x < 0             ||
        d->rooms[i].position.y + d->rooms[i].size.y > DUNGEON_Y - 1 ||
        d->rooms[i].position.y + d->rooms[i].size.y < 0)             {
      fprintf(stderr, "Invalid room position in restored dungeon.\n");

      exit(-1);
    }


    /* After reading each room, we need to reconstruct them in the dungeon. */
    for (y = d->rooms[i].position.y;
         y < d->rooms[i].position.y + d->rooms[i].size.y;
         y++) {
      for (x = d->rooms[i].position.x;
           x < d->rooms[i].position.x + d->rooms[i].size.x;
           x++) {
        mapxy(x, y) = ter_floor_room;
      }
    }
  }

  return 0;
}

int calculate_num_rooms(uint32_t dungeon_bytes)
{
  return ((dungeon_bytes -
          (20 /* The semantic, version, and size */       +
           (DUNGEON_X * DUNGEON_Y) /* The hardnesses */)) /
          4 /* Four bytes per room */);
}

int read_dungeon(dungeon_t *d, char *file)
{
  char semantic[sizeof (DUNGEON_SAVE_SEMANTIC)];
  uint32_t be32;
  FILE *f;
  const char *home;
  size_t len;
  char *filename;
  #ifdef __linux__
  struct stat buf;
  #endif

  if (!file) {
    #ifdef __linux__
    if (!(home = getenv("HOME"))) {
      fprintf(stderr, "\"HOME\" is undefined.  Using working directory.\n");
      home = ".";
    }
    #else
    // get the size of the localappdata path
    DWORD env_size = GetEnvironmentVariable("LOCALAPPDATA", NULL, 0);

    // no app data
    if(!env_size) {
      fprintf(stderr, "\"LOCALAPPDATA\" is undefined.  Using working directory.\n");
      home = strdup(".");
    }
    // get the variable
    else {
      home = (char*) malloc(env_size * sizeof(char));

      GetEnvironmentVariable("LOCALAPPDATA", home, env_size * sizeof(char));
    }
    #endif

    len = (strlen(home) + strlen(SAVE_DIR) + strlen(DUNGEON_SAVE_FILE) +
           1 /* The NULL terminator */                                 +
           2 /* The slashes */);

    filename = (char*) malloc(len * sizeof (*filename));
    sprintf(filename, "%s/%s/%s", home, SAVE_DIR, DUNGEON_SAVE_FILE);

    if (!(f = fopen(filename, "rb"))) {
      perror(filename);
      free(filename);
      exit(-1);
    }

    #ifdef __linux__
    if (stat(filename, &buf)) {
      perror(filename);
      exit(-1);
    }
    #else
    free(home);
    #endif

    free(filename);
  } else {
    if (!(f = fopen(file, "rb"))) {
      perror(file);
      exit(-1);
    }
    #ifdef __linux__
    if (stat(file, &buf)) {
      perror(file);
      exit(-1);
    }
    #endif
  }

  d->num_rooms = 0;

  fread(semantic, sizeof (DUNGEON_SAVE_SEMANTIC) - 1, 1, f);
  semantic[sizeof (DUNGEON_SAVE_SEMANTIC) - 1] = '\0';
  if (strncmp(semantic, DUNGEON_SAVE_SEMANTIC,
	      sizeof (DUNGEON_SAVE_SEMANTIC) - 1)) {
    fprintf(stderr, "Not an RLG327 save file.\n");
    exit(-1);
  }
  fread(&be32, sizeof (be32), 1, f);
  if (be32toh(be32) != 0) { /* Since we expect zero, be32toh() is a no-op. */
    fprintf(stderr, "File version mismatch.\n");
    exit(-1);
  }
  fread(&be32, sizeof (be32), 1, f);
  int num_rooms = be32toh(be32);
  #ifdef __linux__
  if (buf.st_size != num_rooms) {
    fprintf(stderr, "File size mismatch.\n");
    exit(-1);
  }
  #endif
  read_dungeon_map(d, f);
  d->num_rooms = calculate_num_rooms(num_rooms);
  d->rooms = (room_t*) malloc(sizeof (*d->rooms) * d->num_rooms);
  read_rooms(d, f);

  fclose(f);

  return 0;
}

int read_pgm(dungeon_t *d, char *pgm)
{
  FILE *f;
  char s[80];
  uint8_t gm[DUNGEON_Y - 2][DUNGEON_X - 2];
  uint32_t x, y;
  uint32_t i;
  char size[8]; /* Big enough to hold two 3-digit values with a space between. */

  if (!(f = fopen(pgm, "r"))) {
    perror(pgm);
    exit(-1);
  }

  if (!fgets(s, 80, f) || strncmp(s, "P5", 2)) {
    fprintf(stderr, "Expected P5\n");
    exit(-1);
  }
  if (!fgets(s, 80, f) || s[0] != '#') {
    fprintf(stderr, "Expected comment\n");
    exit(-1);
  }
  snprintf(size, 8, "%d %d", DUNGEON_X - 2, DUNGEON_Y - 2);
  if (!fgets(s, 80, f) || strncmp(s, size, 5)) {
    fprintf(stderr, "Expected %s\n", size);
    exit(-1);
  }
  if (!fgets(s, 80, f) || strncmp(s, "255", 2)) {
    fprintf(stderr, "Expected 255\n");
    exit(-1);
  }

  fread(gm, 1, (DUNGEON_X - 2) * (DUNGEON_Y - 2), f);

  fclose(f);

  /* In our gray map, treat black (0) as corridor, white (255) as room, *
   * all other values as a hardness.  For simplicity, treat every white *
   * cell as its own room, so we have to count white after reading the  *
   * image in order to allocate the room array.                         */
  for (d->num_rooms = 0, y = 0; y < DUNGEON_Y - 2; y++) {
    for (x = 0; x < DUNGEON_X - 2; x++) {
      if (!gm[y][x]) {
        d->num_rooms++;
      }
    }
  }
  d->rooms = (room_t*) malloc(sizeof (*d->rooms) * d->num_rooms);

  for (i = 0, y = 0; y < DUNGEON_Y - 2; y++) {
    for (x = 0; x < DUNGEON_X - 2; x++) {
      if (!gm[y][x]) {
        d->rooms[i].position.x = x + 1;
        d->rooms[i].position.y = y + 1;
        d->rooms[i].size.x = 1;
        d->rooms[i].size.y = 1;
        i++;
        d->map[y + 1][x + 1] = ter_floor_room;
        d->hardness[y + 1][x + 1] = 0;
      } else if (gm[y][x] == 255) {
        d->map[y + 1][x + 1] = ter_floor_hall;
        d->hardness[y + 1][x + 1] = 0;
      } else {
        d->map[y + 1][x + 1] = ter_wall;
        d->hardness[y + 1][x + 1] = gm[y][x];
      }
    }
  }

  for (x = 0; x < DUNGEON_X; x++) {
    d->map[0][x] = ter_wall_immutable;
    d->hardness[0][x] = 255;
    d->map[DUNGEON_Y - 1][x] = ter_wall_immutable;
    d->hardness[DUNGEON_Y - 1][x] = 255;
  }
  for (y = 1; y < DUNGEON_Y - 1; y++) {
    d->map[y][0] = ter_wall_immutable;
    d->hardness[y][0] = 255;
    d->map[y][DUNGEON_X - 1] = ter_wall_immutable;
    d->hardness[y][DUNGEON_X - 1] = 255;
  }

  return 0;
}

void render_distance_map(dungeon_t *d)
{
  pair_t p;

  for (p.y = 0; p.y < DUNGEON_Y; p.y++) {
    for (p.x = 0; p.x < DUNGEON_X; p.x++) {
      if (p.x ==  d->pc.position.x &&
          p.y ==  d->pc.position.y) {
        putchar('@');
      } else {
        switch (mappair(p)) {
        case ter_wall:
        case ter_wall_immutable:
          putchar(' ');
          break;
        case ter_staircase_down:
        case ter_staircase_up:
        case ter_floor:
        case ter_floor_room:
        case ter_floor_hall:
          if (d->pc_distance[p.y][p.x] == 255) {
            /* Display an asteric for distance infinity */
            putchar('*');
          } else {
            putchar('0' + d->pc_distance[p.y][p.x] % 10);
          }
          break;
        case ter_debug:
          fprintf(stderr, "Debug character at %d, %d\n", p.y, p.x);
          putchar('*');
          break;
        }
      }
    }
    putchar('\n');
  }
}

void render_tunnel_distance_map(dungeon_t *d)
{
  pair_t p;

  for (p.y = 0; p.y < DUNGEON_Y; p.y++) {
    for (p.x = 0; p.x < DUNGEON_X; p.x++) {
      if (p.x ==  d->pc.position.x &&
          p.y ==  d->pc.position.y) {
        putchar('@');
      } else {
        switch (mappair(p)) {
        case ter_staircase_down:
          mvaddch(p.y, p.x, '>');
          break;
        case ter_staircase_up:
          mvaddch(p.y, p.x, '<');
          break;
        case ter_wall_immutable:
          putchar(' ');
          break;
        case ter_wall:
        case ter_floor:
        case ter_floor_room:
        case ter_floor_hall:
          if (d->pc_tunnel[p.y][p.x] == 255) {
            /* Display an asteric for distance infinity */
            putchar('*');
          } else {
            putchar('0' + d->pc_tunnel[p.y][p.x] % 10);
          }
          break;
        case ter_debug:
          fprintf(stderr, "Debug character at %d, %d\n", p.y, p.x);
          putchar('*');
          break;
        }
      }
    }
    putchar('\n');
  }
}

void place_stairs(dungeon_t *d) {
  // rooms for stair cases
  uint32_t stairX1, stairY1, stairX2, stairY2;

  do {
    stairX1 = rand_range(0, DUNGEON_X - 1);
    stairY1 = rand_range(0, DUNGEON_Y - 1);
  } while(hardnessxy(stairX1, stairY1) != 0 ||
          d->character[stairY1][stairX1]);

  do {
    stairX2 = rand_range(0, DUNGEON_X - 1);
    stairY2 = rand_range(0, DUNGEON_Y - 1);
  } while(hardnessxy(stairX2, stairY2) != 0 ||
          (stairX2 != stairX1 && stairY2 != stairY1) ||
          d->character[stairY2][stairX2]);

  mapxy(stairX1, stairY1) = ter_staircase_down;
  mapxy(stairX2, stairY2) = ter_staircase_up;
}
