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
      // TODO: Replace this
      /*
      delete_dungeon(d);
      init_dungeon(d);
      gen_dungeon(d);
      config_pc(d);
      gen_monsters(d);
      place_stairs(d);
      */

      d->mappair(d->pc->position) = key == '<' ? terrain_type_t::staircase_down : terrain_type_t::staircase_up;

      return true;

    case 'm':
      list_monsters(d);
      d->render_dungeon();
      goto top;

    case '?': case '/':
      help();
      d->render_dungeon();
      goto top;

    default:
      mprintf("%c is not a valid command. (press ? for help)", key);
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
