// Based on Jeremy's solution
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

bool pc_t::next_pos(pair_t &dir) {
  dir.x = 0;
  dir.y = 0;

  char key;

  top:
  switch((key = getch())) {
    case '8': case 'k': case 'w':
      dir.y = -1;
      break;

    case '2': case 'j': case 's':
      dir.y = 1;
      break;

    case '1': case 'b':
      dir.y = 1;
      dir.x = -1;
      break;

    case '4': case 'h': case 'a':
      dir.x = -1;
      break;

    case '7': case 'y':
      dir.x = -1;
      dir.y = -1;
      break;

    case '6': case 'l': case 'd':
      dir.x = 1;
      break;

    case '3': case 'n':
      dir.x = 1;
      dir.y = 1;
      break;

    case '9': case 'u':
      dir.x = 1;
      dir.y = -1;
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
      // TODO: fix this
      d->regenerate();

      d->mappair(d->pc->position) = key == '<' ? terrain_type_t::staircase_down : terrain_type_t::staircase_up;

      return true;

    case 'm':
      list_monsters(d);
      render_dungeon();
      goto top;

    case 'f':
      is_fogged = !is_fogged;
      break;

    case '?': case '/':
      help();
      render_dungeon();
      goto top;

    case 't':
      if(!teleporing) {
        d->charpair(position) = nullptr;
        charpair(position) = nullptr;
        teleport_target = position;
        teleporing = true;
      }
      else {
        teleporing = false;
        position = teleport_target;
        d->charpair(position) = this;
        if (d->mappair(position) <= terrain_type_t::floor) {
          d->mappair(position) = terrain_type_t::floor_hall;
        }
      }

      break;

    default:
      mprintf("%c is not a valid command. (press ? for help)", key);
      goto top;
  }

  if(teleporing) {
    teleport_target += dir;
    dir = pair_t();
    d->render_dungeon();
    goto top;
  }

  return false;
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
  p.y -= 3;

  int32_t startX = p.x - 3;

  if(startX < 0) startX = 0;
  if(p.y < 0) p.y = 0;

  for(; p.y < DUNGEON_Y && p.y < position.y + 3; ++p.y) {
    for(p.x = startX; p.x < DUNGEON_X && p.x < position.x + 3; ++p.x) {
      charpair(p) = d->charpair(p);
      mappair(p) = d->mappair(p);
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
