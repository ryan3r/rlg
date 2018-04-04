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

void pc_t::next_pos(pair_t &next) {
  char key;
  next = position;

  top:
  switch((key = getch())) {
    case '8': case 'k': case 'w':
      if(d->hardnessxy(next.x, next.y - 1) < (teleporing ? 255 : 1))
        --next.y;
      break;

    case '2': case 'j': case 's':
      if(d->hardnessxy(next.x, next.y + 1) < (teleporing ? 255 : 1))
        ++next.y;
      break;

    case '1': case 'b':
      if(d->hardnessxy(next.x, next.y + 1) < (teleporing ? 255 : 1))
        ++next.y;
      if(d->hardnessxy(next.x - 1, next.y) < (teleporing ? 255 : 1))
        --next.x;
      break;

    case '4': case 'h': case 'a':
      if(d->hardnessxy(next.x - 1, next.y) < (teleporing ? 255 : 1))
        --next.x;
      break;

    case '7': case 'y':
      if(d->hardnessxy(next.x - 1, next.y) < (teleporing ? 255 : 1))
        --next.x;
      if(d->hardnessxy(next.x, next.y - 1) < (teleporing ? 255 : 1))
        --next.y;
      break;

    case '6': case 'l': case 'd':
      if(d->hardnessxy(next.x + 1, next.y) < (teleporing ? 255 : 1))
        ++next.x;
      break;

    case '3': case 'n':
      if(d->hardnessxy(next.x + 1, next.y) < (teleporing ? 255 : 1))
        ++next.x;
      if(d->hardnessxy(next.x, next.y + 1) < (teleporing ? 255 : 1))
        ++next.y;
      break;

    case '9': case 'u':
      if(d->hardnessxy(next.x + 1, next.y) < (teleporing ? 255 : 1))
        ++next.x;
      if(d->hardnessxy(next.x, next.y - 1) < (teleporing ? 255 : 1))
        --next.y;
      break;

    case '5': case ' ':
      break;

    case 'Q':
      endwin();
      exit(0);

    case '<':
    case '>':
      if(d->mappair(d->pc->position) != (key == '>' ? terrain_type_t::staircase_down : terrain_type_t::staircase_up)) {
        mprintf("You are not on a%s staircase.", key == '>' ? " down" : "n up");
        goto top;
      }

      // regenerate the entire dungeon
      d->regenerate();
	  return;

    case 'm':
      list_monsters(d);
      render_dungeon();
      goto top;

#ifdef CHEETS
    case 'f':
      is_fogged = !is_fogged;
      render_dungeon();
      goto top;
#endif

    case '?': case '/':
      help();
      render_dungeon();
      goto top;

#ifdef CHEETS
    case 't':
      // enter teleporting mode
      if(!teleporing) {
        d->charpair(position) = nullptr;
        teleport_target = position;
        teleporing = true;
      }
      // exit teleporting mode and move the pc
      else {
        teleporing = false;
        position = teleport_target;
        d->charpair(position) = this;

        if (d->mappair(position) <= terrain_type_t::floor) {
          d->mappair(position) = terrain_type_t::floor_hall;
        }

        look_around();
        render_dungeon();
        goto top;
      }

      break;

    case 'r':
      if(teleporing) {
        teleporing = false;

        next.x = rand_range(1, DUNGEON_X - 2);
        next.y = rand_range(1, DUNGEON_Y - 2);

        position = next;

        d->charpair(position) = this;

        if (d->mappair(position) <= terrain_type_t::floor) {
          d->mappair(position) = terrain_type_t::floor_hall;
        }

        look_around();
        render_dungeon();
        goto top;
      }
#endif

    default:
      mprintf("%c is not a valid command. (press ? for help)", key);
      goto top;
  }

#ifdef CHEETS
  // move the cursor and rerender
  if(teleporing) {
    teleport_target = next;
    d->render_dungeon();
    goto top;
  }
#endif
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

void pc_t::render_dungeon() {
  if(!is_fogged) {
    d->render_dungeon();
    return;
  }

  erase();

  pair_t p;

  for (p.y = 0; p.y < DUNGEON_Y; p.y++) {
    for (p.x = 0; p.x < DUNGEON_X; p.x++) {
      if(p == teleport_target && teleporing) {
        attron(COLOR_PAIR(1));
        mvaddch(p.y + 1, p.x, '*');
        attroff(COLOR_PAIR(1));
      }
      else if (d->charpair(p) && can_see(p) && -VISUAL_DISTANCE <= d->pc->position.x - p.x && d->pc->position.x - p.x <= VISUAL_DISTANCE &&
            d->pc->position.y - p.y <= VISUAL_DISTANCE && d->pc->position.y - p.y >= -VISUAL_DISTANCE) {
        int color = d->charpair(p)->symbol == '@' ?
			1 : resolve_color(((npc_t*) d->charpair(p))->color[0]);

        attron(COLOR_PAIR(color));
        mvaddch(p.y + 1, p.x, d->charpair(p)->symbol);
        attroff(COLOR_PAIR(color));
      }
	  else if (d->objpair(p) && can_see(p) && -VISUAL_DISTANCE <= d->pc->position.x - p.x && d->pc->position.x - p.x <= VISUAL_DISTANCE &&
		  d->pc->position.y - p.y <= VISUAL_DISTANCE && d->pc->position.y - p.y >= -VISUAL_DISTANCE) {
		  int color = resolve_color(d->objpair(p)->color[0]);

		  attron(COLOR_PAIR(color));
		  mvaddch(p.y + 1, p.x, d->objpair(p)->symbol());
		  attroff(COLOR_PAIR(color));
	  }
	  else {
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
