// Based on Jeremy's solution for 1.04
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

uint32_t max_monster_cells(dungeon_t *d)
{
  uint32_t i;
  uint32_t sum;

  for (i = sum = 0; i < d->rooms.size(); i++) {
    if (!d->pc->in_room(i)) {
      sum += d->rooms[i].size.y * d->rooms[i].size.x;
    }
  }

  return sum;
}

inline uint16_t min(uint16_t a, uint16_t b) {
    return a < b ? a : b;
}

void npc_t::gen_monsters(dungeon_t *d)
{
  uint32_t i;
  character_t *m;
  uint32_t room;
  pair_t p;

  d->num_monsters = min((uint32_t) d->max_monsters, max_monster_cells(d));

  for (i = 0; i < d->num_monsters; i++) {
    do {
      room = rand_range(1, d->rooms.size() - 1);
      p.y = rand_range(d->rooms[room].position.y,
                            (d->rooms[room].position.y +
                             d->rooms[room].size.y - 1));
      p.x = rand_range(d->rooms[room].position.x,
                            (d->rooms[room].position.x +
                             d->rooms[room].size.x - 1));
    } while (d->charpair(p));

    m = new npc_t(d);
    m->position = p;
    d->charpair(p) = m;

    heap_insert(&d->events, new_event(d, event_character_turn, m, 0));
  }
}

void npc_t::next_pos_rand_tunnel(pair_t &next)
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

void npc_t::next_pos_rand(pair_t &next)
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

void npc_t::next_pos_line_of_sight(pair_t &next)
{
  pair_t dir;

  dir.y = d->pc->position.y - position.y;
  dir.x = d->pc->position.x - position.x;
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

void npc_t::next_pos_line_of_sight_tunnel(pair_t &next)
{
  pair_t dir;

  dir.y = d->pc->position.y - position.y;
  dir.x = d->pc->position.x - position.x;
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

void npc_t::next_pos_gradient(pair_t &next)
{
  /* Handles both tunneling and non-tunneling versions */
  pair_t min_next;
  uint16_t min_cost;
  if (has_attr(npc_t::TUNNEL)) {
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

void npc_t::next_pos_00(pair_t &next)
{
  /* not smart; not telepathic; not tunneling; not erratic */
  if (can_see(d->pc)) {
    pc_last_known_position.y = d->pc->position.y;
    pc_last_known_position.x = d->pc->position.x;
    next_pos_line_of_sight(next);
  } else {
    next_pos_rand(next);
  }
}

void npc_t::next_pos_01(pair_t &next)
{
  /*     smart; not telepathic; not tunneling; not erratic */
  if (can_see(d->pc)) {
    pc_last_known_position.y = d->pc->position.y;
    pc_last_known_position.x = d->pc->position.x;
    have_seen_pc = 1;
    next_pos_line_of_sight(next);
  } else if (have_seen_pc) {
    next_pos_line_of_sight(next);
  }

  if ((next.x == pc_last_known_position.x) &&
      (next.y == pc_last_known_position.y)) {
    have_seen_pc = 0;
  }
}

void npc_t::next_pos_02(pair_t &next)
{
  /* not smart;     telepathic; not tunneling; not erratic */
  pc_last_known_position.y = d->pc->position.y;
  pc_last_known_position.x = d->pc->position.x;
  next_pos_line_of_sight(next);
}

void npc_t::next_pos_03(pair_t &next)
{
  /*     smart;     telepathic; not tunneling; not erratic */
  next_pos_gradient(next);
}

void npc_t::next_pos_04(pair_t &next)
{
  /* not smart; not telepathic;     tunneling; not erratic */
  if (can_see(d->pc)) {
    pc_last_known_position.y = d->pc->position.y;
    pc_last_known_position.x = d->pc->position.x;
    next_pos_line_of_sight(next);
  } else {
    next_pos_rand_tunnel(next);
  }
}

void npc_t::next_pos_05(pair_t &next)
{
  /*     smart; not telepathic;     tunneling; not erratic */
  if (can_see(d->pc)) {
    pc_last_known_position.y = d->pc->position.y;
    pc_last_known_position.x = d->pc->position.x;
    have_seen_pc = 1;
    next_pos_line_of_sight(next);
  } else if (have_seen_pc) {
    next_pos_line_of_sight_tunnel(next);
  }

  if ((next.x == pc_last_known_position.x) &&
      (next.y == pc_last_known_position.y)) {
    have_seen_pc = 0;
  }
}

void npc_t::next_pos_06(pair_t &next)
{
  /* not smart;     telepathic;     tunneling; not erratic */
  pc_last_known_position.y = d->pc->position.y;
  pc_last_known_position.x = d->pc->position.x;
  next_pos_line_of_sight_tunnel(next);
}

void npc_t::next_pos_07(pair_t &next)
{
  /*     smart;     telepathic;     tunneling; not erratic */
  next_pos_gradient(next);
}

void npc_t::next_pos_08(pair_t &next)
{
  /* not smart; not telepathic; not tunneling;     erratic */
  if (rand() & 1) {
    next_pos_rand(next);
  } else {
    next_pos_00(next);
  }
}

void npc_t::next_pos_09(pair_t &next)
{
  /*     smart; not telepathic; not tunneling;     erratic */
  if (rand() & 1) {
    next_pos_rand(next);
  } else {
    next_pos_01(next);
  }
}

void npc_t::next_pos_0a(pair_t &next)
{
  /* not smart;     telepathic; not tunneling;     erratic */
  if (rand() & 1) {
    next_pos_rand(next);
  } else {
    next_pos_02(next);
  }
}

void npc_t::next_pos_0b(pair_t &next)
{
  /*     smart;     telepathic; not tunneling;     erratic */
  if (rand() & 1) {
    next_pos_rand(next);
  } else {
    next_pos_03(next);
  }
}

void npc_t::next_pos_0c(pair_t &next)
{
  /* not smart; not telepathic;     tunneling;     erratic */
  if (rand() & 1) {
    next_pos_rand_tunnel(next);
  } else {
    next_pos_04(next);
  }
}

void npc_t::next_pos_0d(pair_t &next)
{
  /*     smart; not telepathic;     tunneling;     erratic */
  if (rand() & 1) {
    next_pos_rand_tunnel(next);
  } else {
    next_pos_05(next);
  }
}

void npc_t::next_pos_0e(pair_t &next)
{
  /* not smart;     telepathic;     tunneling;     erratic */
  if (rand() & 1) {
    next_pos_rand_tunnel(next);
  } else {
    next_pos_06(next);
  }
}

void npc_t::next_pos_0f(pair_t &next)
{
  /*     smart;     telepathic;     tunneling;     erratic */
  if (rand() & 1) {
    next_pos_rand_tunnel(next);
  } else {
    next_pos_07(next);
  }
}

void npc_t::next_pos(pair_t &next)
{
  next = position;

  if(attrs == 0x00) next_pos_00(next);
	if(attrs == 0x01) next_pos_01(next);
	if(attrs == 0x02) next_pos_02(next);
	if(attrs == 0x03) next_pos_03(next);
	if(attrs == 0x04) next_pos_04(next);
	if(attrs == 0x05) next_pos_05(next);
	if(attrs == 0x06) next_pos_06(next);
	if(attrs == 0x07) next_pos_07(next);
	if(attrs == 0x08) next_pos_08(next);
	if(attrs == 0x09) next_pos_09(next);
	if(attrs == 0x0a) next_pos_0a(next);
	if(attrs == 0x0b) next_pos_0b(next);
	if(attrs == 0x0c) next_pos_0c(next);
	if(attrs == 0x0d) next_pos_0d(next);
	if(attrs == 0x0e) next_pos_0e(next);
	if(attrs == 0x0f) next_pos_0f(next);
}
