// Based on Jeremy's solution for 1.04
#include <stdlib.h>

#include <character.hpp>
#include <heap.h>
#include <pc.hpp>
#include <dungeon.hpp>
#include <iostream>

uint32_t character_t::can_see(const pair_t &target) const {
  /* Application of Bresenham's Line Drawing Algorithm.  If we can draw *
   * a line from v to e without intersecting any walls, then v can see  *
   * e.  Unfortunately, Bresenham isn't symmetric, so line-of-sight     *
   * based on this approach is not reciprocal (Helmholtz Reciprocity).  *
   * This is a very real problem in roguelike games, and one we're      *
   * going to ignore for now.  Algorithms that are symmetrical are far  *
   * more expensive.                                                    */

  pair_t first, second;
  pair_t del, f;
  int16_t a, b, c, i;

  first = position;
  second = target;

  if ((abs(first.x - second.x) > VISUAL_RANGE) ||
      (abs(first.y - second.y) > VISUAL_RANGE)) {
    return 0;
  }

  /*
  d->mappair(first) = terrain_type_t::debug;
  d->mappair(second) = terrain_type_t::debug;
  */

  if (second.x > first.x) {
    del.x = second.x - first.x;
    f.x = 1;
  } else {
    del.x = first.x - second.x;
    f.x = -1;
  }

  if (second.y > first.y) {
    del.y = second.y - first.y;
    f.y = 1;
  } else {
    del.y = first.y - second.y;
    f.y = -1;
  }

  if (del.x > del.y) {
    a = del.y + del.y;
    c = a - del.x;
    b = c - del.x;
    for (i = 0; i <= del.x; i++) {
      if ((d->mappair(first) < terrain_type_t::floor) && i && (i != del.x)) {
        return 0;
      }
      /*      d->mappair(first) = terrain_type_t::debug;*/
      first.x += f.x;
      if (c < 0) {
        c += a;
      } else {
        c += b;
        first.y += f.y;
      }
    }
    return 1;
  } else {
    a = del.x + del.x;
    c = a - del.y;
    b = c - del.y;
    for (i = 0; i <= del.y; i++) {
      if ((d->mappair(first) < terrain_type_t::floor) && i && (i != del.y)) {
        return 0;
      }
      /*      d->mappair(first) = terrain_type_t::debug;*/
      first.y += f.y;
      if (c < 0) {
        c += a;
      } else {
        c += b;
        first.x += f.x;
      }
    }
    return 1;
  }

  return 1;
}

void character_t::attack(character_t &def) const {
	((pc_t&) def).defend(*this);
}

int32_t character_t::get_speed() const {
	return speed;
}