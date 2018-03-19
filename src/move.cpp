// Based on Jeremy's solution
#include <move.hpp>

#ifdef __linux__
#include <ncurses.h>
#else
#include <curses.h>
#endif

#include <stdlib.h>
#include <assert.h>

#include <dungeon.hpp>
#include<heap.h>
#include <move.hpp>
#include <npc.hpp>
#include <pc.hpp>
#include <character.hpp>
#include <utils.hpp>
#include <path.hpp>
#include <event.hpp>

void do_combat(dungeon_t *d, character_t *atk, character_t *def)
{
  if (def->alive) {
    def->alive = 0;
    if (def != &d->pc) {
      d->num_monsters--;
    }
    atk->kills[kill_direct]++;
    atk->kills[kill_avenged] += (def->kills[kill_direct] +
                                  def->kills[kill_avenged]);
  }
}

void move_character(dungeon_t *d, character_t *c, pair_t &next)
{
  if (charpair(next) &&
      ((next.y != c->position.y) ||
       (next.x != c->position.x))) {
    do_combat(d, c, charpair(next));
  } else {
    /* No character in new position. */

    d->character[c->position.y][c->position.x] = NULL;
    c->position.y = next.y;
    c->position.x = next.x;
    d->character[c->position.y][c->position.x] = c;
  }
}

void do_moves(dungeon_t *d)
{
  pair_t next;
  character_t *c;
  event_t *e;

  /* Remove the PC when it is PC turn.  Replace on next call.  This allows *
   * use to completely uninit the heap when generating a new level without *
   * worrying about deleting the PC.                                       */

  if (pc_is_alive(d)) {
    /* The PC always goes first one a tie, so we don't use new_event().  *
     * We generate one manually so that we can set the PC sequence       *
     * number to zero.                                                   */
    e = (event_t*) malloc(sizeof (*e));
    e->type = event_character_turn;
    /* Hack: New dungeons are marked.  Unmark and ensure PC goes at d->time, *
     * otherwise, monsters get a turn before the PC.                         */
    if (d->is_new) {
      d->is_new = 0;
      e->time = d->time;
    } else {
      e->time = d->time + (1000 / d->pc.speed);
    }
    e->sequence = 0;
    e->c = &d->pc;
    heap_insert(&d->events, e);
  }

  while (pc_is_alive(d) &&
         (e = (event_t*) heap_remove_min(&d->events)) &&
         ((e->type != event_character_turn) || (e->c != &d->pc))) {
    d->time = e->time;
    if (e->type == event_character_turn) {
      c = e->c;
    }
    if (!c->alive) {
      if (d->character[c->position.y][c->position.x] == c) {
        d->character[c->position.y][c->position.x] = NULL;
      }
      if (c != &d->pc) {
        event_delete(e);
      }
      continue;
    }

    npc_next_pos(d, c, next);
    move_character(d, c, next);

    heap_insert(&d->events, update_event(d, e, 1000 / c->speed));
  }

  if (pc_is_alive(d) && e->c == &d->pc) {
    c = e->c;
    d->time = e->time;
    /* Kind of kludgey, but because the PC is never in the queue when   *
     * we are outside of this function, the PC event has to get deleted *
     * and recreated every time we leave and re-enter this function.    */
    e->c = NULL;
    event_delete(e);
    if(pc_next_pos(d, next)) return;

    // don't go into hardnesses above 254
    if(hardnessxy(next.x + c->position.x, c->position.y) < 255) {
      next.x += c->position.x;
    }
    else {
       next.x = c->position.x;
    }

    if(hardnessxy(c->position.x, next.y + c->position.y) < 255) {
      next.y += c->position.y;
    }
    else {
       next.y = c->position.y;
    }

    if (mappair(next) <= ter_floor) {
      mappair(next) = ter_floor_hall;
    }
    move_character(d, c, next);

    dijkstra(d);
    dijkstra_tunnel(d);
  }
}

void dir_nearest_wall(dungeon_t *d, character_t *c, pair_t &dir)
{
  dir.x = dir.y = 0;

  if (c->position.x != 1 && c->position.x != DUNGEON_X - 2) {
    dir.x = (c->position.x > DUNGEON_X - c->position.x ? 1 : -1);
  }
  if (c->position.y != 1 && c->position.y != DUNGEON_Y - 2) {
    dir.y = (c->position.y > DUNGEON_Y - c->position.y ? 1 : -1);
  }
}

uint32_t against_wall(dungeon_t *d, character_t *c)
{
  return ((mapxy(c->position.x - 1,
                 c->position.y    ) == ter_wall_immutable) ||
          (mapxy(c->position.x + 1,
                 c->position.y    ) == ter_wall_immutable) ||
          (mapxy(c->position.x    ,
                 c->position.y - 1) == ter_wall_immutable) ||
          (mapxy(c->position.x    ,
                 c->position.y + 1) == ter_wall_immutable));
}

uint32_t in_corner(dungeon_t *d, character_t *c)
{
  uint32_t num_immutable;

  num_immutable = 0;

  num_immutable += (mapxy(c->position.x - 1,
                          c->position.y    ) == ter_wall_immutable);
  num_immutable += (mapxy(c->position.x + 1,
                          c->position.y    ) == ter_wall_immutable);
  num_immutable += (mapxy(c->position.x    ,
                          c->position.y - 1) == ter_wall_immutable);
  num_immutable += (mapxy(c->position.x    ,
                          c->position.y + 1) == ter_wall_immutable);

  return num_immutable > 1;
}

uint32_t move_pc(dungeon_t *d, uint32_t dir)
{
  return 1;
}
