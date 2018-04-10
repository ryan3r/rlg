// Based on Jeremy's solution for 1.04
#include <stdlib.h>
#include <string.h>

#ifdef __linux__
#include <ncurses.h>
#else
#include <curses.h>
#endif

#include <dungeon.hpp>
#include <pc.hpp>
#include <npc.hpp>
#include <utils.hpp>
#include <move.hpp>
#include <path.hpp>
#include <info.hpp>
#include <event.hpp>
#include <iostream>

void pc_t::place_pc() {
  position.y = rand_range(d->rooms[0].position.y,
                                     (d->rooms[0].position.y +
                                      d->rooms[0].size.y - 1));
  position.x = rand_range(d->rooms[0].position.x,
                                     (d->rooms[0].position.x +
                                      d->rooms[0].size.x - 1));
}

void pc_t::config_pc() {
  place_pc();

  d->charpair(position) = this;

  heap_insert(&d->events, new_event(d, event_character_turn, this, 0));

  dijkstra(d);
  dijkstra_tunnel(d);
}

// handle keys to move the character/targeting pointer
bool pc_t::move_keys(char key, pair_t &next) {
	switch (key) {
		case '8': case 'k': case 'w':
			if (d->hardnessxy(next.x, next.y - 1) < (teleporting ? 255 : 1))
				--next.y;
			break;

		case '2': case 'j': case 's':
			if (d->hardnessxy(next.x, next.y + 1) < (teleporting ? 255 : 1))
				++next.y;
			break;

		case '1': case 'b':
			if (d->hardnessxy(next.x, next.y + 1) < (teleporting ? 255 : 1))
				++next.y;
			if (d->hardnessxy(next.x - 1, next.y) < (teleporting ? 255 : 1))
				--next.x;
			break;

		case '4': case 'h': case 'a':
			if (d->hardnessxy(next.x - 1, next.y) < (teleporting ? 255 : 1))
				--next.x;
			break;

		case '7': case 'y':
			if (d->hardnessxy(next.x - 1, next.y) < (teleporting ? 255 : 1))
				--next.x;
			if (d->hardnessxy(next.x, next.y - 1) < (teleporting ? 255 : 1))
				--next.y;
			break;

		case '6': case 'l': case 'd':
			if (d->hardnessxy(next.x + 1, next.y) < (teleporting ? 255 : 1))
				++next.x;
			break;

		case '3': case 'n':
			if (d->hardnessxy(next.x + 1, next.y) < (teleporting ? 255 : 1))
				++next.x;
			if (d->hardnessxy(next.x, next.y + 1) < (teleporting ? 255 : 1))
				++next.y;
			break;

		case '9': case 'u':
			if (d->hardnessxy(next.x + 1, next.y) < (teleporting ? 255 : 1))
				++next.x;
			if (d->hardnessxy(next.x, next.y - 1) < (teleporting ? 255 : 1))
				--next.y;
			break;

		case '5': case ' ':
			break;

		default:
			return false;
	}

	return true;
}

void pc_t::target(char exit_key, std::function<uint8_t(char)> handler) {
	bool should_render = true;
	teleporting = true;
	teleport_target = position;

	for (;;) {
		if(should_render) d->render_dungeon();
		should_render = true;

		char key = getch();

		// move the target
		if (!move_keys(key, teleport_target)) {
			// finish targeting
			if (key == exit_key) break;
			// cancel targeting
			else if (key == 'Q' || key == 27) {
				teleport_target = position;
				break;
			}
			// pass on the key to the handler
			else {
				if (handler == nullptr) {
					mprintf("%c is not a valid command. (press ? for help)", key);
					should_render = false;
				}
				else {
					uint8_t res = handler(key);
					
					// exit targeting
					if (res == 1) break;

					// unknown key
					if (res == 2) {
						mprintf("%c is not a valid command. (press ? for help)", key);
						should_render = false;
					}
				}
			}
		}
	}

	teleporting = false;
	d->render_dungeon();
}

void pc_t::next_pos(pair_t &next) {
	char key;
	bool __is_fogged;
	next = position;

	top:
	key = getch();

	if (!move_keys(key, next)) {
		switch (key) {
		case 'Q':
			d->pc->alive = false;
			break;

		case '<':
		case '>':
			if (d->mappair(d->pc->position) != (key == '>' ? terrain_type_t::staircase_down : terrain_type_t::staircase_up)) {
				mprintf("You are not on a%s staircase.", key == '>' ? " down" : "n up");
				goto top;
			}

			// regenerate the entire dungeon
			regenerate_dungeon = true;
			return;

		case 'm':
			list_monsters(d);
			d->render_dungeon();
			goto top;

		case '?': case '/':
			help();
			d->render_dungeon();
			goto top;

	#ifdef CHEETS
		case 'f':
			is_fogged = !is_fogged;
			d->render_dungeon();
			goto top;

		case 'g':
			d->charpair(position) = nullptr;
			// save the fogged state
			__is_fogged = is_fogged;
			is_fogged = false;

			// target the teleporting location
			target('g', [&](char key) -> uint8_t {
				if (key == 'r') {
					teleport_target.x = rand_range(1, DUNGEON_X - 2);
					teleport_target.y = rand_range(1, DUNGEON_Y - 2);
					return 1;
				}
				else {
					return 2;
				}

				return 0;
			});

			// place the pc
			is_fogged = __is_fogged;
			position = next = teleport_target;

			d->charpair(position) = this;

			if (d->mappair(next) <= terrain_type_t::floor) {
				d->mappair(next) = terrain_type_t::floor_hall;
			}

			look_around();
			d->render_dungeon();
		  
			goto top;
	#endif

		default:
			mprintf("%c is not a valid command. (press ? for help)", key);
			goto top;
		}
	}
}

bool pc_t::in_room(uint32_t room) {
  if ((room < d->rooms.size()) &&
      (position.x >= d->rooms[room].position.x) &&
      (position.x < (d->rooms[room].position.x +
                                d->rooms[room].size.x))    &&
      (position.y >= d->rooms[room].position.y) &&
      (position.y < (d->rooms[room].position.y +
                                d->rooms[room].size.y))) {
    return true;
  }

  return false;
}

void pc_t::look_around() {
  pair_t p = position;
  p.y -= VISUAL_DISTANCE;

  int32_t startX = p.x - VISUAL_DISTANCE;

  if(startX < 0) startX = 0;
  if(p.y < 0) p.y = 0;

  for(; p.y < DUNGEON_Y && p.y < position.y + VISUAL_DISTANCE; ++p.y) {
    for(p.x = startX; p.x < DUNGEON_X && p.x < position.x + VISUAL_DISTANCE; ++p.x) {
      if(can_see(p)) {
        mappair(p) = d->mappair(p);
      }
    }
  }
}
