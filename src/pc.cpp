// Based on Jeremy's solution
#include <stdlib.h>
#include<string.h>

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

void pc_delete(pc_t *pc)
{
  if (pc) {
    free(pc);
  }
}

uint32_t pc_is_alive(dungeon_t *d)
{
  return d->pc.alive;
}

void place_pc(dungeon_t *d)
{
  d->pc.position.y = rand_range(d->rooms->position.y,
                                     (d->rooms->position.y +
                                      d->rooms->size.y - 1));
  d->pc.position.x = rand_range(d->rooms->position.x,
                                     (d->rooms->position.x +
                                      d->rooms->size.x - 1));
}

void config_pc(dungeon_t *d)
{
  memset(&d->pc, 0, sizeof (d->pc));
  d->pc.symbol = '@';

  place_pc(d);

  d->pc.speed = PC_SPEED;
  d->pc.alive = 1;
  d->pc.sequence_number = 0;
  d->pc.pc = (pc_t*) calloc(1, sizeof (*d->pc.pc));
  d->pc.npc = NULL;
  d->pc.kills[kill_direct] = d->pc.kills[kill_avenged] = 0;

  d->character[d->pc.position.y][d->pc.position.x] = &d->pc;

  dijkstra(d);
  dijkstra_tunnel(d);
}

uint32_t pc_next_pos(dungeon_t *d, pair_t &dir)
{
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
      if(mappair(d->pc.position) != (key == '>' ? ter_staircase_down : ter_staircase_up)) {
        mprintf("You are not on a%s staircase.", key == '>' ? " down" : "n up");
        goto top;
      }

      // regenerate the entire dungeon
      delete_dungeon(d);
      init_dungeon(d);
      gen_dungeon(d);
      config_pc(d);
      gen_monsters(d);
      place_stairs(d);

      mappair(d->pc.position) = key == '<' ? ter_staircase_down : ter_staircase_up;

      return 1;

    case 'm':
      list_monsters(d);
      render_dungeon(d);
      goto top;

    case '?': case '/':
      help();
      render_dungeon(d);
      goto top;

    default:
      mprintf("%c is not a valid command. (press ? for help)", key);
      goto top;
  }

  return 0;
}

uint32_t pc_in_room(dungeon_t *d, uint32_t room)
{
  if ((room < d->num_rooms)                                     &&
      (d->pc.position.x >= d->rooms[room].position.x) &&
      (d->pc.position.x < (d->rooms[room].position.x +
                                d->rooms[room].size.x))    &&
      (d->pc.position.y >= d->rooms[room].position.y) &&
      (d->pc.position.y < (d->rooms[room].position.y +
                                d->rooms[room].size.y))) {
    return 1;
  }

  return 0;
}
