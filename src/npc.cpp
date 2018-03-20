// Based on Jeremy's solution
#include <stdlib.h>

#include <utils.hpp>
#include <npc.hpp>
#include <dungeon.hpp>
#include <character.hpp>
#include <move.hpp>
#include <path.hpp>
#include <event.hpp>
#include <pc.hpp>
#include <string.h>

void npc_delete(npc_t *n)
{
  if (n) {
    free(n);
  }
}

static uint32_t max_monster_cells(dungeon_t *d)
{
  uint32_t i;
  uint32_t sum;

  for (i = sum = 0; i < d->rooms.size(); i++) {
    if (!pc_in_room(d, i)) {
      sum += d->rooms[i].size.y * d->rooms[i].size.x;
    }
  }

  return sum;
}

void gen_monsters(dungeon_t *d)
{
  uint32_t i;
  character_t *m;
  uint32_t room;
  pair_t p;
  const static char symbol[] = "0123456789abcdef";

  d->num_monsters = min((uint32_t) d->max_monsters, max_monster_cells(d));

  for (i = 0; i < d->num_monsters; i++) {
    m = (character_t*) malloc(sizeof (*m));
    memset(m, 0, sizeof (*m));

    do {
      room = rand_range(1, d->rooms.size() - 1);
      p.y = rand_range(d->rooms[room].position.y,
                            (d->rooms[room].position.y +
                             d->rooms[room].size.y - 1));
      p.x = rand_range(d->rooms[room].position.x,
                            (d->rooms[room].position.x +
                             d->rooms[room].size.x - 1));
    } while (d->charpair(p));
    m->position.y = p.y;
    m->position.x = p.x;
    d->charpair(p) = m;
    m->speed = rand_range(5, 20);
    m->alive = 1;
    m->sequence_number = ++d->character_sequence_number;
    m->pc = NULL;
    m->npc = (npc_t*) malloc(sizeof (*m->npc));
    m->npc->characteristics = rand() & 0x0000000f;
    /*    m->npc->characteristics = 0xf;*/
    m->symbol = symbol[m->npc->characteristics];
    m->npc->have_seen_pc = 0;
    m->kills[kill_direct] = m->kills[kill_avenged] = 0;

    d->charpair(p) = m;

    heap_insert(&d->events, new_event(d, event_character_turn, m, 0));
  }
}

void npc_next_pos_rand_tunnel(dungeon_t *d, character_t *c, pair_t &next)
{
  pair_t n;
  union {
    uint32_t i;
    uint8_t a[4];
  } r;

  do {
    n.y = next.y;
    n.x = next.x;
    r.i = rand();
    if (r.a[0] > 85 /* 255 / 3 */) {
      if (r.a[0] & 1) {
        n.y--;
      } else {
        n.y++;
      }
    }
    if (r.a[1] > 85 /* 255 / 3 */) {
      if (r.a[1] & 1) {
        n.x--;
      } else {
        n.x++;
      }
    }
  } while (d->mappair(n) == terrain_type_t::wall_immutable);

  if (d->hardnesspair(n) <= 85) {
    if (d->hardnesspair(n)) {
      d->hardnesspair(n) = 0;
      d->mappair(n) = terrain_type_t::floor_hall;

      /* Update distance maps because map has changed. */
      dijkstra(d);
      dijkstra_tunnel(d);
    }

    next.x = n.x;
    next.y = n.y;
  } else {
    d->hardnesspair(n) -= 85;
  }
}

void npc_next_pos_rand(dungeon_t *d, character_t *c, pair_t &next)
{
  pair_t n;
  union {
    uint32_t i;
    uint8_t a[4];
  } r;

  do {
    n.y = next.y;
    n.x = next.x;
    r.i = rand();
    if (r.a[0] > 85 /* 255 / 3 */) {
      if (r.a[0] & 1) {
        n.y--;
      } else {
        n.y++;
      }
    }
    if (r.a[1] > 85 /* 255 / 3 */) {
      if (r.a[1] & 1) {
        n.x--;
      } else {
        n.x++;
      }
    }
  } while (d->mappair(n) < terrain_type_t::floor);

  next.y = n.y;
  next.x = n.x;
}

void npc_next_pos_line_of_sight(dungeon_t *d, character_t *c, pair_t &next)
{
  pair_t dir;

  dir.y = d->pc->position.y - c->position.y;
  dir.x = d->pc->position.x - c->position.x;
  if (dir.y) {
    dir.y /= abs(dir.y);
  }
  if (dir.x) {
    dir.x /= abs(dir.x);
  }

  if (d->mapxy(next.x + dir.x,
            next.y + dir.y) >= terrain_type_t::floor) {
    next.x += dir.x;
    next.y += dir.y;
  } else if (d->mapxy(next.x + dir.x, next.y) >= terrain_type_t::floor) {
    next.x += dir.x;
  } else if (d->mapxy(next.x, next.y + dir.y) >= terrain_type_t::floor) {
    next.y += dir.y;
  }
}

void npc_next_pos_line_of_sight_tunnel(dungeon_t *d,
                                       character_t *c,
                                       pair_t &next)
{
  pair_t dir;

  dir.y = d->pc->position.y - c->position.y;
  dir.x = d->pc->position.x - c->position.x;
  if (dir.y) {
    dir.y /= abs(dir.y);
  }
  if (dir.x) {
    dir.x /= abs(dir.x);
  }

  dir.x += next.x;
  dir.y += next.y;

  if (d->hardnesspair(dir) <= 60) {
    if (d->hardnesspair(dir)) {
      d->hardnesspair(dir) = 0;
      d->mappair(dir) = terrain_type_t::floor_hall;

      /* Update distance maps because map has changed. */
      dijkstra(d);
      dijkstra_tunnel(d);
    }

    next.x = dir.x;
    next.y = dir.y;
  } else {
    d->hardnesspair(dir) -= 60;
  }
}

void npc_next_pos_gradient(dungeon_t *d, character_t *c, pair_t &next)
{
  /* Handles both tunneling and non-tunneling versions */
  pair_t min_next;
  uint16_t min_cost;
  if (c->npc->characteristics & NPC_TUNNEL) {
    min_cost = (d->pc_tunnel[next.y - 1][next.x] +
                (d->hardnessxy(next.x, next.y - 1) / 60));
    min_next.x = next.x;
    min_next.y = next.y - 1;
    if ((d->pc_tunnel[next.y + 1][next.x    ] +
         (d->hardnessxy(next.x, next.y + 1) / 60)) < min_cost) {
      min_cost = (d->pc_tunnel[next.y + 1][next.x] +
                  (d->hardnessxy(next.x, next.y + 1) / 60));
      min_next.x = next.x;
      min_next.y = next.y + 1;
    }
    if ((d->pc_tunnel[next.y    ][next.x + 1] +
         (d->hardnessxy(next.x + 1, next.y) / 60)) < min_cost) {
      min_cost = (d->pc_tunnel[next.y][next.x + 1] +
                  (d->hardnessxy(next.x + 1, next.y) / 60));
      min_next.x = next.x + 1;
      min_next.y = next.y;
    }
    if ((d->pc_tunnel[next.y    ][next.x - 1] +
         (d->hardnessxy(next.x - 1, next.y) / 60)) < min_cost) {
      min_cost = (d->pc_tunnel[next.y][next.x - 1] +
                  (d->hardnessxy(next.x - 1, next.y) / 60));
      min_next.x = next.x - 1;
      min_next.y = next.y;
    }
    if ((d->pc_tunnel[next.y - 1][next.x + 1] +
         (d->hardnessxy(next.x + 1, next.y - 1) / 60)) < min_cost) {
      min_cost = (d->pc_tunnel[next.y - 1][next.x + 1] +
                  (d->hardnessxy(next.x + 1, next.y - 1) / 60));
      min_next.x = next.x + 1;
      min_next.y = next.y - 1;
    }
    if ((d->pc_tunnel[next.y + 1][next.x + 1] +
         (d->hardnessxy(next.x + 1, next.y + 1) / 60)) < min_cost) {
      min_cost = (d->pc_tunnel[next.y + 1][next.x + 1] +
                  (d->hardnessxy(next.x + 1, next.y + 1) / 60));
      min_next.x = next.x + 1;
      min_next.y = next.y + 1;
    }
    if ((d->pc_tunnel[next.y - 1][next.x - 1] +
         (d->hardnessxy(next.x - 1, next.y - 1) / 60)) < min_cost) {
      min_cost = (d->pc_tunnel[next.y - 1][next.x - 1] +
                  (d->hardnessxy(next.x - 1, next.y - 1) / 60));
      min_next.x = next.x - 1;
      min_next.y = next.y - 1;
    }
    if ((d->pc_tunnel[next.y + 1][next.x - 1] +
         (d->hardnessxy(next.x - 1, next.y + 1) / 60)) < min_cost) {
      min_cost = (d->pc_tunnel[next.y + 1][next.x - 1] +
                  (d->hardnessxy(next.x - 1, next.y + 1) / 60));
      min_next.x = next.x - 1;
      min_next.y = next.y + 1;
    }
    if (d->hardnesspair(min_next) <= 60) {
      if (d->hardnesspair(min_next)) {
        d->hardnesspair(min_next) = 0;
        d->mappair(min_next) = terrain_type_t::floor_hall;

        /* Update distance maps because map has changed. */
        dijkstra(d);
        dijkstra_tunnel(d);
      }

      next.x = min_next.x;
      next.y = min_next.y;
    } else {
      d->hardnesspair(min_next) -= 60;
    }
  } else {
    /* Make monsters prefer cardinal directions */
    if (d->pc_distance[next.y - 1][next.x    ] <
        d->pc_distance[next.y][next.x]) {
      next.y--;
      return;
    }
    if (d->pc_distance[next.y + 1][next.x    ] <
        d->pc_distance[next.y][next.x]) {
      next.y++;
      return;
    }
    if (d->pc_distance[next.y    ][next.x + 1] <
        d->pc_distance[next.y][next.x]) {
      next.x++;
      return;
    }
    if (d->pc_distance[next.y    ][next.x - 1] <
        d->pc_distance[next.y][next.x]) {
      next.x--;
      return;
    }
    if (d->pc_distance[next.y - 1][next.x + 1] <
        d->pc_distance[next.y][next.x]) {
      next.y--;
      next.x++;
      return;
    }
    if (d->pc_distance[next.y + 1][next.x + 1] <
        d->pc_distance[next.y][next.x]) {
      next.y++;
      next.x++;
      return;
    }
    if (d->pc_distance[next.y - 1][next.x - 1] <
        d->pc_distance[next.y][next.x]) {
      next.y--;
      next.x--;
      return;
    }
    if (d->pc_distance[next.y + 1][next.x - 1] <
        d->pc_distance[next.y][next.x]) {
      next.y++;
      next.x--;
      return;
    }
  }
}

static void npc_next_pos_00(dungeon_t *d, character_t *c, pair_t &next)
{
  /* not smart; not telepathic; not tunneling; not erratic */
  if (can_see(d, c, d->pc)) {
    c->npc->pc_last_known_position.y = d->pc->position.y;
    c->npc->pc_last_known_position.x = d->pc->position.x;
    npc_next_pos_line_of_sight(d, c, next);
  } else {
    npc_next_pos_rand(d, c, next);
  }
}

static void npc_next_pos_01(dungeon_t *d, character_t *c, pair_t &next)
{
  /*     smart; not telepathic; not tunneling; not erratic */
  if (can_see(d, c, d->pc)) {
    c->npc->pc_last_known_position.y = d->pc->position.y;
    c->npc->pc_last_known_position.x = d->pc->position.x;
    c->npc->have_seen_pc = 1;
    npc_next_pos_line_of_sight(d, c, next);
  } else if (c->npc->have_seen_pc) {
    npc_next_pos_line_of_sight(d, c, next);
  }

  if ((next.x == c->npc->pc_last_known_position.x) &&
      (next.y == c->npc->pc_last_known_position.y)) {
    c->npc->have_seen_pc = 0;
  }
}

static void npc_next_pos_02(dungeon_t *d, character_t *c, pair_t &next)
{
  /* not smart;     telepathic; not tunneling; not erratic */
  c->npc->pc_last_known_position.y = d->pc->position.y;
  c->npc->pc_last_known_position.x = d->pc->position.x;
  npc_next_pos_line_of_sight(d, c, next);
}

static void npc_next_pos_03(dungeon_t *d, character_t *c, pair_t &next)
{
  /*     smart;     telepathic; not tunneling; not erratic */
  npc_next_pos_gradient(d, c, next);
}

static void npc_next_pos_04(dungeon_t *d, character_t *c, pair_t &next)
{
  /* not smart; not telepathic;     tunneling; not erratic */
  if (can_see(d, c, d->pc)) {
    c->npc->pc_last_known_position.y = d->pc->position.y;
    c->npc->pc_last_known_position.x = d->pc->position.x;
    npc_next_pos_line_of_sight(d, c, next);
  } else {
    npc_next_pos_rand_tunnel(d, c, next);
  }
}

static void npc_next_pos_05(dungeon_t *d, character_t *c, pair_t &next)
{
  /*     smart; not telepathic;     tunneling; not erratic */
  if (can_see(d, c, d->pc)) {
    c->npc->pc_last_known_position.y = d->pc->position.y;
    c->npc->pc_last_known_position.x = d->pc->position.x;
    c->npc->have_seen_pc = 1;
    npc_next_pos_line_of_sight(d, c, next);
  } else if (c->npc->have_seen_pc) {
    npc_next_pos_line_of_sight_tunnel(d, c, next);
  }

  if ((next.x == c->npc->pc_last_known_position.x) &&
      (next.y == c->npc->pc_last_known_position.y)) {
    c->npc->have_seen_pc = 0;
  }
}

static void npc_next_pos_06(dungeon_t *d, character_t *c, pair_t &next)
{
  /* not smart;     telepathic;     tunneling; not erratic */
  c->npc->pc_last_known_position.y = d->pc->position.y;
  c->npc->pc_last_known_position.x = d->pc->position.x;
  npc_next_pos_line_of_sight_tunnel(d, c, next);
}

static void npc_next_pos_07(dungeon_t *d, character_t *c, pair_t &next)
{
  /*     smart;     telepathic;     tunneling; not erratic */
  npc_next_pos_gradient(d, c, next);
}

static void npc_next_pos_08(dungeon_t *d, character_t *c, pair_t &next)
{
  /* not smart; not telepathic; not tunneling;     erratic */
  if (rand() & 1) {
    npc_next_pos_rand(d, c, next);
  } else {
    npc_next_pos_00(d, c, next);
  }
}

static void npc_next_pos_09(dungeon_t *d, character_t *c, pair_t &next)
{
  /*     smart; not telepathic; not tunneling;     erratic */
  if (rand() & 1) {
    npc_next_pos_rand(d, c, next);
  } else {
    npc_next_pos_01(d, c, next);
  }
}

static void npc_next_pos_0a(dungeon_t *d, character_t *c, pair_t &next)
{
  /* not smart;     telepathic; not tunneling;     erratic */
  if (rand() & 1) {
    npc_next_pos_rand(d, c, next);
  } else {
    npc_next_pos_02(d, c, next);
  }
}

static void npc_next_pos_0b(dungeon_t *d, character_t *c, pair_t &next)
{
  /*     smart;     telepathic; not tunneling;     erratic */
  if (rand() & 1) {
    npc_next_pos_rand(d, c, next);
  } else {
    npc_next_pos_03(d, c, next);
  }
}

static void npc_next_pos_0c(dungeon_t *d, character_t *c, pair_t &next)
{
  /* not smart; not telepathic;     tunneling;     erratic */
  if (rand() & 1) {
    npc_next_pos_rand_tunnel(d, c, next);
  } else {
    npc_next_pos_04(d, c, next);
  }
}

static void npc_next_pos_0d(dungeon_t *d, character_t *c, pair_t &next)
{
  /*     smart; not telepathic;     tunneling;     erratic */
  if (rand() & 1) {
    npc_next_pos_rand_tunnel(d, c, next);
  } else {
    npc_next_pos_05(d, c, next);
  }
}

static void npc_next_pos_0e(dungeon_t *d, character_t *c, pair_t &next)
{
  /* not smart;     telepathic;     tunneling;     erratic */
  if (rand() & 1) {
    npc_next_pos_rand_tunnel(d, c, next);
  } else {
    npc_next_pos_06(d, c, next);
  }
}

static void npc_next_pos_0f(dungeon_t *d, character_t *c, pair_t &next)
{
  /*     smart;     telepathic;     tunneling;     erratic */
  if (rand() & 1) {
    npc_next_pos_rand_tunnel(d, c, next);
  } else {
    npc_next_pos_07(d, c, next);
  }
}

void (*npc_move_func[])(dungeon_t *d, character_t *c, pair_t &next) = {
  /* We'll have one function for each combination of bits, so the *
   * order is based on binary counting through the NPC_* bits.    *
   * It could be very easy to mess this up, so be careful.  We'll *
   * name them according to their hex value.                      */
  npc_next_pos_00,
  npc_next_pos_01,
  npc_next_pos_02,
  npc_next_pos_03,
  npc_next_pos_04,
  npc_next_pos_05,
  npc_next_pos_06,
  npc_next_pos_07,
  npc_next_pos_08,
  npc_next_pos_09,
  npc_next_pos_0a,
  npc_next_pos_0b,
  npc_next_pos_0c,
  npc_next_pos_0d,
  npc_next_pos_0e,
  npc_next_pos_0f,
};

void npc_next_pos(dungeon_t *d, character_t *c, pair_t &next)
{
  next.y = c->position.y;
  next.x = c->position.x;

  npc_move_func[c->npc->characteristics & 0x0000000f](d, c, next);
}

uint32_t dungeon_has_npcs(dungeon_t *d)
{
  return d->num_monsters;
}
