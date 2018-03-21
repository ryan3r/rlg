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
#include <string.h>

#include <dungeon.hpp>
#include <utils.hpp>
#include <heap.h>
#include <event.hpp>
#include <character.hpp>
#include <npc.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#define DUMP_HARDNESS_IMAGES 0

typedef struct corridor_path {
  heap_node_t *hn;
  pair_t pos;
  pair_t from;
  int32_t cost;
} corridor_path_t;

/* Will need this later.
static uint32_t in_room(dungeon_t *int16_t y, int16_t x)
{
  int i;

  for (i = 0; i < num_rooms; i++) {
    if ((x >= rooms[i].position.x) &&
        (x < (rooms[i].position.x + rooms[i].size.x)) &&
        (y >= rooms[i].position.y) &&
        (y < (rooms[i].position.y + rooms[i].size.y))) {
      return 1;
    }
  }

  return 0;
}
*/

int32_t dungeon_t::adjacent_to_room(int32_t y, int32_t x)
{
  return (mapxy(x - 1, y) == terrain_type_t::floor_room ||
          mapxy(x + 1, y) == terrain_type_t::floor_room ||
          mapxy(x, y - 1) == terrain_type_t::floor_room ||
          mapxy(x, y + 1) == terrain_type_t::floor_room);
}

int32_t corridor_path_cmp(const void *key, const void *with) {
  return ((corridor_path_t *) key)->cost - ((corridor_path_t *) with)->cost;
}

void dungeon_t::dijkstra_corridor(const pair_t &from, const pair_t &to)
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
      if (mapxy(x, y) != terrain_type_t::wall_immutable) {
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
        if (mapxy(x, y) != terrain_type_t::floor_room) {
          mapxy(x, y) = terrain_type_t::floor_hall;
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
void dungeon_t::dijkstra_corridor_inv(const pair_t &from, const pair_t &to)
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
      if (mapxy(x, y) != terrain_type_t::wall_immutable) {
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
        if (mapxy(x, y) != terrain_type_t::floor_room) {
          mapxy(x, y) = terrain_type_t::floor_hall;
          hardnessxy(x, y) = 0;
        }
      }
      heap_delete(&h);
      return;
    }

#define hardnesspair_inv(p) (is_open_space(p.y, p.x) ? 127 :     \
                             (adjacent_to_room(p.y, p.x) ? 191 : \
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
void dungeon_t::connect_two_rooms(const room_t *r1, const room_t *r2)
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

  dijkstra_corridor(e1, e2);
}

void dungeon_t::create_cycle()
{
  /* Find the (approximately) farthest two rooms, then connect *
   * them by the shortest path using inverted hardnesses.      */

  int32_t max, tmp, i, j, p, q;
  pair_t e1, e2;

  for (i = max = 0; i < rooms.size() - 1; i++) {
    for (j = i + 1; j < rooms.size(); j++) {
      tmp = (((rooms[i].position.x - rooms[j].position.x)  *
              (rooms[i].position.x - rooms[j].position.x)) +
             ((rooms[i].position.y - rooms[j].position.y)  *
              (rooms[i].position.y - rooms[j].position.y)));
      if (tmp > max) {
        max = tmp;
        p = i;
        q = j;
      }
    }
  }

  /* Can't simply call connect_two_rooms() because it doesn't *
   * use inverse hardnesses, so duplicate it here.            */
  e1.y = rand_range(rooms[p].position.y,
                         (rooms[p].position.y +
                          rooms[p].size.y - 1));
  e1.x = rand_range(rooms[p].position.x,
                         (rooms[p].position.x +
                          rooms[p].size.x - 1));
  e2.y = rand_range(rooms[q].position.y,
                         (rooms[q].position.y +
                          rooms[q].size.y - 1));
  e2.x = rand_range(rooms[q].position.x,
                         (rooms[q].position.x +
                          rooms[q].size.x - 1));

  dijkstra_corridor_inv(e1, e2);
}

void dungeon_t::connect_rooms()
{
  uint32_t i;

  for (i = 1; i < rooms.size(); i++) {
    connect_two_rooms(&rooms[i - 1], &rooms[i]);
  }

  create_cycle();
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

void dungeon_t::smooth_hardness()
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
      hardnessxy(x, y) = t / s;
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
      hardnessxy(x, y) = t / s;
    }
  }
}

void dungeon_t::empty_dungeon()
{
  uint8_t x, y;

  smooth_hardness();
  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      mapxy(x, y) = terrain_type_t::wall;
      if (y == 0 || y == DUNGEON_Y - 1 ||
          x == 0 || x == DUNGEON_X - 1) {
        mapxy(x, y) = terrain_type_t::wall_immutable;
        hardnessxy(x, y) = 255;
      }
    }
  }

  is_new = 1;
}

void dungeon_t::place_rooms()
{
  pair_t p;
  uint32_t i;
  int success;
  room_t *r;

  for (success = 0; !success; ) {
    success = 1;
    for (i = 0; success && i < rooms.size(); i++) {
      r = &rooms[i];
      r->position.x = 1 + rand() % (DUNGEON_X - 2 - r->size.x);
      r->position.y = 1 + rand() % (DUNGEON_Y - 2 - r->size.y);
      for (p.y = r->position.y - 1;
           success && p.y < r->position.y + r->size.y + 1;
           p.y++) {
        for (p.x = r->position.x - 1;
             success && p.x < r->position.x + r->size.x + 1;
             p.x++) {
          if (mappair(p) >= terrain_type_t::floor) {
            success = 0;

            empty_dungeon();
          } else if ((p.y != r->position.y - 1)              &&
                     (p.y != r->position.y + r->size.y) &&
                     (p.x != r->position.x - 1)              &&
                     (p.x != r->position.x + r->size.x)) {
            mappair(p) = terrain_type_t::floor_room;
            hardnesspair(p) = 0;
          }
        }
      }
    }
  }
}

void dungeon_t::make_rooms()
{
  uint32_t i;
  room_t room;

  for (i = 0;  i < MIN_ROOMS || (i < MAX_ROOMS && rand_under(6, 8)); i++) {
    room.size.x = ROOM_MIN_X;
    room.size.y = ROOM_MIN_Y;
    while (rand_under(3, 4) && room.size.x < ROOM_MAX_X) {
      room.size.x++;
    }
    while (rand_under(3, 4) && room.size.y < ROOM_MAX_Y) {
      room.size.y++;
    }
    rooms.push_back(room);
  }
}

void dungeon_t::gen_dungeon()
{
  empty_dungeon();
  make_rooms();
  place_rooms();
  connect_rooms();

  init();
}

void dungeon_t::render_dungeon() {
  erase();

  pair_t p;

  for (p.y = 0; p.y < DUNGEON_Y; p.y++) {
    for (p.x = 0; p.x < DUNGEON_X; p.x++) {
      if(p == pc->teleport_target && pc->teleporing) {
        attron(COLOR_PAIR(1));
        mvaddch(p.y + 1, p.x, '*');
        attroff(COLOR_PAIR(1));
      }
      else if (charpair(p)) {
        int color = (charpair(p)->symbol != '@') + 1;

        attron(COLOR_PAIR(color));
        mvaddch(p.y + 1, p.x, charpair(p)->symbol);
        attroff(COLOR_PAIR(color));
      } else {
        switch (mappair(p)) {
        case terrain_type_t::staircase_down:
          attron(COLOR_PAIR(3));
          mvaddch(p.y + 1, p.x, '>');
          attroff(COLOR_PAIR(3));
          break;
        case terrain_type_t::staircase_up:
          attron(COLOR_PAIR(3));
          mvaddch(p.y + 1, p.x, '<');
          attroff(COLOR_PAIR(3));
          break;
        case terrain_type_t::wall:
        case terrain_type_t::wall_immutable:
          mvaddch(p.y + 1, p.x, ' ');
          break;
        case terrain_type_t::floor:
        case terrain_type_t::floor_room:
          mvaddch(p.y + 1, p.x, '.');
          break;
        case terrain_type_t::floor_hall:
          mvaddch(p.y + 1, p.x, '#');
          break;
        case terrain_type_t::debug:
          mvaddch(p.y + 1, p.x, '*');
          mprintf("Debug character at %d, %d\n", p.y, p.x);
          break;
        }
      }
    }
  }
}

dungeon_t::dungeon_t()
{
  empty_dungeon();
  memset(&character, 0, sizeof(character));
  memset(&events, 0, sizeof (events));
  heap_init(&events, compare_events, event_delete);
  pc = new pc_t(this);
}

void dungeon_t::init() {
  pc->config_pc();
  npc_t::gen_monsters(this);
  place_stairs();
}

void dungeon_t::regenerate() {
  int step = 0;

  rooms.empty();
  heap_delete(&events);

  empty_dungeon();

  memset(&character, 0, sizeof(character));
  memset(&events, 0, sizeof (events));

  heap_init(&events, compare_events, event_delete);

  pc = new pc_t(this);

  gen_dungeon();

}

void dungeon_t::write_dungeon_map(std::ostream &out) {
  int32_t x, y;

  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      out.write((const char*) &hardness[y][x], sizeof(uint8_t));
    }
  }
}

void dungeon_t::write_rooms(std::ostream &out) {
  uint8_t p;

  for(auto room : rooms) {
    /* write order is xpos, ypos, width, height */
    p = room.position.y;
    out.write((const char*) &p, 1);
    p = room.position.x;
    out.write((const char*) &p, 1);
    p = room.size.y;
    out.write((const char*) &p, 1);
    p = room.size.x;
    out.write((const char*) &p, 1);
  }
}

uint32_t dungeon_t::calculate_dungeon_size() {
  return (20 /* The semantic, version, and size */     +
          (DUNGEON_X * DUNGEON_Y) /* The hardnesses */ +
          (rooms.size() * 4) /* Four bytes per room */);
}

#ifdef _WIN32
int htobe32(int x) { return x; }
int be32toh(int x) { return x; }
#endif

std::string get_default_file(const char *target) {
  std::stringstream filename_s;

  const char *home;
  #ifdef __linux__
  if (!(home = getenv("HOME"))) {
    std::cerr << "\"HOME\" is undefined.  Using working directory." << std::endl;
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
    home = new char[env_size];

    GetEnvironmentVariable("LOCALAPPDATA", home, env_size * sizeof(char));
  }
  #endif

  filename_s << home << "/" << SAVE_DIR;

  char *path = strdup(filename_s.str().c_str());
  makedirectory(path);
  delete path;

  filename_s << "/" << target;

  #ifndef __linux__
  free(home);
  #endif

  return filename_s.str();
}

void dungeon_t::write_dungeon() {
  write_dungeon(get_default_file(DUNGEON_SAVE_FILE));
}

void dungeon_t::write_dungeon(std::string file) {
  std::ofstream f(file, std::ios::out | std::ios::binary);

  if(f.fail()) {
    // TODO: Turn into an exception
    perror(file.c_str());
    exit(-1);
  }

  /* The semantic, which is 6 bytes, 0-5 */
  f.write(DUNGEON_SAVE_SEMANTIC, sizeof (DUNGEON_SAVE_SEMANTIC) - 1);

  /* The version, 4 bytes, 6-9 */
  int be32 = htobe32(DUNGEON_SAVE_VERSION);
  f.write((const char*) &be32, sizeof(be32));

  /* The size of the file, 4 bytes, 10-13 */
  be32 = htobe32(calculate_dungeon_size());
  f.write((const char*) &be32, sizeof(be32));

  /* The dungeon map, 1680 bytes, 14-1693 */
  write_dungeon_map(f);

  /* And the rooms, num_rooms * 4 bytes, 1694-end */
  write_rooms(f);

  f.close();
}

void dungeon_t::read_dungeon_map(std::istream &in) {
  pair_t p;

  for (; p.y < DUNGEON_Y; p.y++) {
    for (p.x = 0; p.x < DUNGEON_X; p.x++) {
      in.read((char*) &hardness[p.y][p.x], 1);
      if (hardnesspair(p) == 0) {
        /* Mark it as a corridor.  We can't recognize room cells until *
         * after we've read the room array, which we haven't done yet. */
        mappair(p) = terrain_type_t::floor_hall;
      } else if (hardness[p.y][p.x] == 255) {
        mappair(p) = terrain_type_t::wall_immutable;
      } else {
        mappair(p) = terrain_type_t::wall;
      }
    }
  }
}

void dungeon_t::read_rooms(std::istream &in, int num_rooms) {
  int32_t x, y;
  char p;
  room_t room;

  for(int i = 0; i < num_rooms; ++i) {
    in.read((char*) &p, 1);
    room.position.y = p;
    in.read((char*) &p, 1);
    room.position.x = p;
    in.read((char*) &p, 1);
    room.size.y = p;
    in.read((char*) &p, 1);
    room.size.x = p;

    rooms.push_back(room);

    if (room.size.x < 1             ||
        room.size.y < 1             ||
        room.size.x > DUNGEON_X - 1 ||
        room.size.y > DUNGEON_X - 1) {
      // TODO: Replace with exception
      std::cerr << "Invalid room size in restored dungeon." << std::endl;

      exit(-1);
    }

    if (room.position.x < 1                                       ||
        room.position.y < 1                                       ||
        room.position.x > DUNGEON_X - 1                           ||
        room.position.y > DUNGEON_Y - 1                           ||
        room.position.x + room.size.x > DUNGEON_X - 1 ||
        room.position.x + room.size.x < 0             ||
        room.position.y + room.size.y > DUNGEON_Y - 1 ||
        room.position.y + room.size.y < 0)             {
      // TODO: Replace with exception
      std::cerr << "Invalid room position in restored dungeon." << std::endl;

      exit(-1);
    }

    /* After reading each room, we need to reconstruct them in the dungeon. */
    for (y = room.position.y;
         y < room.position.y + room.size.y;
         y++) {
      for (x = room.position.x;
           x < room.position.x + room.size.x;
           x++) {
        mapxy(x, y) = terrain_type_t::floor_room;
      }
    }
  }
}

int dungeon_t::calculate_num_rooms(uint32_t dungeon_bytes) {
  return ((dungeon_bytes -
          (20 /* The semantic, version, and size */       +
           (DUNGEON_X * DUNGEON_Y) /* The hardnesses */)) /
          4 /* Four bytes per room */);
}

void dungeon_t::read_dungeon() {
  read_dungeon(get_default_file(DUNGEON_SAVE_FILE));
}

void dungeon_t::read_dungeon(std::string file) {
  char semantic[sizeof (DUNGEON_SAVE_SEMANTIC)];

  std::ifstream f(file, std::ios::in | std::ios::binary);

  if (f.fail()) {
    // TODO: Turn into an exception
    perror(file.c_str());
    exit(-1);
  }

  // get the file size
  f.seekg(0, f.end);
  long file_size = f.tellg();
  f.seekg(0);

  // --- SEAM ---- (file_size == -1 if not file)

  f.read(semantic, sizeof (DUNGEON_SAVE_SEMANTIC) - 1);
  semantic[sizeof (DUNGEON_SAVE_SEMANTIC) - 1] = '\0';
  if (strncmp(semantic, DUNGEON_SAVE_SEMANTIC,
	      sizeof (DUNGEON_SAVE_SEMANTIC) - 1)) {
    // TODO: Convert to an exception
    std::cerr << "Not an RLG327 save file." << std::endl;
    exit(-1);
  }

  uint32_t be32;
  f.read((char*) &be32, sizeof (be32));
  if (be32toh(be32) != 0) { /* Since we expect zero, be32toh() is a no-op. */
    // TODO: Convert to an exception
    std::cerr << "File version mismatch." << std::endl;
    exit(-1);
  }
  f.read((char*) &be32, sizeof (be32));
  // verify the size (file_size == -1 if no size is given)
  if (file_size != -1 && file_size != be32toh(be32)) {
    fprintf(stderr, "File size mismatch.\n");
    exit(-1);
  }

  read_dungeon_map(f);
  read_rooms(f, calculate_num_rooms(be32toh(be32)));

  f.close();

  init();
}

void dungeon_t::render_distance_map()
{
  pair_t p;

  for (p.y = 0; p.y < DUNGEON_Y; p.y++) {
    for (p.x = 0; p.x < DUNGEON_X; p.x++) {
      if (p.x ==  pc->position.x &&
          p.y ==  pc->position.y) {
        putchar('@');
      } else {
        switch (mappair(p)) {
        case terrain_type_t::wall:
        case terrain_type_t::wall_immutable:
          putchar(' ');
          break;
        case terrain_type_t::staircase_down:
        case terrain_type_t::staircase_up:
        case terrain_type_t::floor:
        case terrain_type_t::floor_room:
        case terrain_type_t::floor_hall:
          if (pc_distance[p.y][p.x] == 255) {
            /* Display an asteric for distance infinity */
            putchar('*');
          } else {
            putchar('0' + pc_distance[p.y][p.x] % 10);
          }
          break;
        case terrain_type_t::debug:
          fprintf(stderr, "Debug character at %d, %d\n", p.y, p.x);
          putchar('*');
          break;
        }
      }
    }
    putchar('\n');
  }
}

void dungeon_t::render_tunnel_distance_map()
{
  pair_t p;

  for (p.y = 0; p.y < DUNGEON_Y; p.y++) {
    for (p.x = 0; p.x < DUNGEON_X; p.x++) {
      if (p.x ==  pc->position.x &&
          p.y ==  pc->position.y) {
        putchar('@');
      } else {
        switch (mappair(p)) {
        case terrain_type_t::staircase_down:
          mvaddch(p.y, p.x, '>');
          break;
        case terrain_type_t::staircase_up:
          mvaddch(p.y, p.x, '<');
          break;
        case terrain_type_t::wall_immutable:
          putchar(' ');
          break;
        case terrain_type_t::wall:
        case terrain_type_t::floor:
        case terrain_type_t::floor_room:
        case terrain_type_t::floor_hall:
          if (pc_tunnel[p.y][p.x] == 255) {
            /* Display an asteric for distance infinity */
            putchar('*');
          } else {
            putchar('0' + pc_tunnel[p.y][p.x] % 10);
          }
          break;
        case terrain_type_t::debug:
          fprintf(stderr, "Debug character at %d, %d\n", p.y, p.x);
          putchar('*');
          break;
        }
      }
    }
    putchar('\n');
  }
}

void dungeon_t::place_stairs() {
  // rooms for stair cases
  uint32_t stairX1, stairY1, stairX2, stairY2;

  do {
    stairX1 = rand_range(0, DUNGEON_X - 1);
    stairY1 = rand_range(0, DUNGEON_Y - 1);
  } while(hardnessxy(stairX1, stairY1) != 0 ||
          charxy(stairX1, stairY1));

  do {
    stairX2 = rand_range(0, DUNGEON_X - 1);
    stairY2 = rand_range(0, DUNGEON_Y - 1);
  } while(hardnessxy(stairX2, stairY2) != 0 ||
          (stairX2 != stairX1 && stairY2 != stairY1) ||
          charxy(stairX2, stairY2));

  mapxy(stairX1, stairY1) = terrain_type_t::staircase_down;
  mapxy(stairX2, stairY2) = terrain_type_t::staircase_up;
}
