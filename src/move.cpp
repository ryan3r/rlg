// Based on Jeremy's solution for 1.04
#include <move.hpp>

#ifdef __linux__
#include <ncurses.h>
#else
#include <curses.h>
#endif

#include <stdlib.h>
#include <assert.h>

#include <dungeon.hpp>
#include <heap.h>
#include <move.hpp>
#include <npc.hpp>
#include <pc.hpp>
#include <character.hpp>
#include <utils.hpp>
#include <path.hpp>
#include <event.hpp>
#include <iostream>

const pair_t DIRECTIONS[] = {
	pair_t(-1, -1), pair_t(0, -1), pair_t(1, -1),
	pair_t(-1, 0), pair_t(0, 0), pair_t(1, 0),
	pair_t(-1, 1), pair_t(0, 1), pair_t(1, 1)
};

void do_combat(dungeon_t *d, character_t *atk, character_t *def) {
  if (!def->alive()) return;

  if (atk == d->pc || def == d->pc) {
	atk->attack(*def);

	if (!def->alive()) {
		if (def != d->pc) {
			d->num_monsters--;
		}
		atk->kills_direct++;
		atk->kills_avenged += (def->kills_direct + def->kills_avenged);
	}
  }
  else {
	  // push the player to a nearby open space
	  for (int i = rand_range(0, 8), end = i + 9; i < end; ++i) {
		  pair_t to = def->position + DIRECTIONS[i % 9];

		  if (!d->charpair(to) && d->hardnesspair(to) == 0) {
			  move_character(d, def, to);
			  break;
		  }
	  }
  }
}

void move_character(dungeon_t *d, character_t *c, pair_t &next)
{
  if (d->charpair(next) &&
      ((next.y != c->position.y) ||
       (next.x != c->position.x))) {
    do_combat(d, c, d->charpair(next));
  } else {
    /* No character in new position. */

    d->charpair(c->position) = NULL;
    c->position.y = next.y;
    c->position.x = next.x;
    d->charpair(c->position) = c;

	// pick up any objects if we can
	if (c == d->pc && d->objpair(c->position)) {
		for (size_t i = 0; i < NUM_CARRY_SLOTS; ++i) {
			if (d->pc->carry[i] == nullptr) {
				d->pc->carry[i] = d->objpair(d->pc->position);
				d->objpair(d->pc->position) = nullptr;
				break;
			}
		}
	}
  }
}

void do_moves(dungeon_t *d)
{
  pair_t next;
  character_t *c;
  event_t *e;

  while (d->pc->alive() &&
         (e = (event_t*) heap_remove_min(&d->events)) &&
         ((e->type != event_character_turn) || (e->c != d->pc))) {
    d->time = e->time;
    if (e->type == event_character_turn) {
      c = e->c;
    }
    if (!c->alive()) {
      if (d->charpair(c->position) == c) {
        d->charpair(c->position) = NULL;
      }
      if (c != d->pc) {
		npc_t* monster = (npc_t*) c;

		// don't allow any more instances of this monster
		if (monster->has_attr(npc_t::UNIQUE)) {
			auto it = d->monster_builders.begin();

			for (; it != d->monster_builders.end(); ++it) {
				MonsterBuilder *builder = dynamic_cast<MonsterBuilder*>((*it).get());

				if (builder->name == monster->name) {
					break;
				}
			}

			if (it != d->monster_builders.end())
				d->monster_builders.erase(it);
		}
      }
	  event_delete(e);
      continue;
    }

    c->next_pos(next);

    move_character(d, c, next);

    heap_insert(&d->events, update_event(d, e, 1000 / c->get_speed()));
  }

  if (d->pc->alive() && e->c == d->pc) {
    c = e->c;
    d->time = e->time;

    heap_insert(&d->events, update_event(d, e, 1000 / c->get_speed()));

    c->next_pos(next);

	// don't do anything we are regenerating
	if (d->pc->regenerate_dungeon) return;

    move_character(d, c, next);

    dijkstra(d);
    dijkstra_tunnel(d);
  }
}
