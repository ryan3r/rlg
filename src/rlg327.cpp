// Based on Jeremy's solution for 1.04
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef __linux__
#include <sys/time.h>
#include <unistd.h>
#include <ncurses.h>
#else
#include <curses.h>
#undef MOUSE_MOVED
#include <windows.h>
#include <ctime>
#endif

/* Very slow seed: 686846853 */

#include <dungeon.hpp>
#include <pc.hpp>
#include <npc.hpp>
#include <move.hpp>
#include <info.hpp>
#include <parser.hpp>

#include <iostream>
#include <fstream>
#include <path.hpp>
#include <logger.hpp>

const char *victory =
  "                                       o\n"
  "                                      $\"\"$o\n"
  "                                     $\"  $$\n"
  "                                      $$$$\n"
  "                                      o \"$o\n"
  "                                     o\"  \"$\n"
  "                oo\"$$$\"  oo$\"$ooo   o$    \"$    ooo\"$oo  $$$\"o\n"
  "   o o o o    oo\"  o\"      \"o    $$o$\"     o o$\"\"  o$      \"$  "
  "\"oo   o o o o\n"
  "   \"$o   \"\"$$$\"   $$         $      \"   o   \"\"    o\"         $"
  "   \"o$$\"    o$$\n"
  "     \"\"o       o  $          $\"       $$$$$       o          $  ooo"
  "     o\"\"\n"
  "        \"o   $$$$o $o       o$        $$$$$\"       $o        \" $$$$"
  "   o\"\n"
  "         \"\"o $$$$o  oo o  o$\"         $$$$$\"        \"o o o o\"  "
  "\"$$$  $\n"
  "           \"\" \"$\"     \"\"\"\"\"            \"\"$\"            \""
  "\"\"      \"\"\" \"\n"
  "            \"oooooooooooooooooooooooooooooooooooooooooooooooooooooo$\n"
  "             \"$$$$\"$$$$\" $$$$$$$\"$$$$$$ \" \"$$$$$\"$$$$$$\"  $$$\""
  "\"$$$$\n"
  "              $$$oo$$$$   $$$$$$o$$$$$$o\" $$$$$$$$$$$$$$ o$$$$o$$$\"\n"
  "              $\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\""
  "\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"$\n"
  "              $\"                                                 \"$\n"
  "              $\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\""
  "$\"$\"$\"$\"$\"$\"$\"$\n"
  "                                   You win!\n\n";

const char *tombstone =
  "\n                /\"\"\"\"\"/\"\"\"\"\"\"\".\n"
  "               /     /         \\             __\n"
  "              /     /           \\            ||\n"
  "             /____ /   Rest in   \\           ||\n"
  "            |     |    Pieces     |          ||\n"
  "            |     |               |          ||\n"
  "            |     |   A. Luser    |          ||\n"
  "            |     |               |          ||\n"
  "            |     |     * *   * * |         _||_\n"
  "            |     |     *\\/* *\\/* |        | TT |\n"
  "            |     |     *_\\_  /   ...\"\"\"\"\"\"| |"
  "| |.\"\"....\"\"\"\"\"\"\"\".\"\"\n"
  "            |     |         \\/..\"\"\"\"\"...\"\"\""
  "\\ || /.\"\"\".......\"\"\"\"...\n"
  "            |     |....\"\"\"\"\"\"\"........\"\"\"\"\""
  "\"^^^^\".......\"\"\"\"\"\"\"\"..\"\n"
  "            |......\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"......"
  "..\"\"\"\"\"....\"\"\"\"\"..\"\"...\"\"\".\n\n"
  "            You're dead.  Better luck in the next life.\n\n";

void usage(char *name)
{
  fprintf(stderr,
          "Usage: %s [-r|--rand <seed>] [-l|--load [<file>]]\n"
          "          [-s|--save [<file>]] [-i|--image <pgm file>]\n"
          "          [-p|--pc <y> <x>] [-n|--nummon <count>]\n",
          name);

  exit(-1);
}

int main(int argc, char *argv[])
{
	try {
		dungeon_t d(
			Parser::parse_file(resolve_config_file("monster_desc.txt")),
			Parser::parse_file(resolve_config_file("object_desc.txt"))
		);

		pc_t::pc = new pc_t(&d);

#ifdef _WIN32
		// change the console title
		SetConsoleTitle("rlg327");

		// resize the window
		SMALL_RECT window_size = { 0, 0, DUNGEON_X, DUNGEON_Y + 2 };
		SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), 1, &window_size);
#endif

		d.max_monsters = MAX_MONSTERS;

		/* Allows me to generate more than one dungeon *
		 * per second, as opposed to time().           */
#ifdef __linux__
		struct timeval tv;
		gettimeofday(&tv, NULL);
		time_t seed = (tv.tv_usec ^ (tv.tv_sec << 20)) & 0xffffffff;
#else
		time_t seed = time(NULL);
#endif

		srand(seed);

		// initiailize ncurses
		initscr();
		noecho();
		raw();
		cbreak();
		start_color();
		keypad(stdscr, TRUE);
		curs_set(0);

		init_pair(1, COLOR_CYAN, COLOR_BLACK);
		init_pair(2, COLOR_RED, COLOR_BLACK);
		init_pair(3, COLOR_GREEN, COLOR_BLACK);
		init_pair(4, COLOR_BLACK, COLOR_WHITE);
		init_pair(5, COLOR_YELLOW, COLOR_BLACK);
		init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
		init_pair(7, COLOR_WHITE, COLOR_BLACK);
		init_pair(8, COLOR_BLUE, COLOR_BLACK);

	game_start:

		if (argc > 1) {
			d.read_dungeon(argv[1]);
		}
		else {
			d.gen_dungeon();
		}

		if (should_show_help()) {
			pc_t::pc->look_around();
			d.render_dungeon();
			refresh();
			help();
		}

		while (pc_t::pc->alive() && !d.is_boss_dead) {
			pc_t::pc->look_around();
			d.render_dungeon();
			do_moves(&d);

			if (pc_t::pc->regenerate_dungeon) {
				d.regenerate();
				pc_t::pc->regenerate();
			}
		}

		// print the victory/defeat artwork
		erase();

		uint32_t yPos = 0;

		char *msg, *last, *orig;
		orig = msg = last = strdup(pc_t::pc->alive() ? victory : tombstone);

		attron(COLOR_PAIR(pc_t::pc->alive() ? 5 : 2));

		for (; *msg; ++msg) {
			if (*msg == '\n') {
				*msg = '\0';

				mvprintw(yPos++, 0, "%s", last);

				*msg = '\n';
				last = msg + 1;
			}
		}

		attroff(COLOR_PAIR(pc_t::pc->alive() ? 5 : 2));

		free(orig);

		mvprintw(yPos++, 0, "You defended your life in the face of %u deadly beasts.", pc_t::pc->kills_direct);

		attron(COLOR_PAIR(1));
		mvprintw(yPos + 1, 0, "(Q)uit, (P)lay again");
		attroff(COLOR_PAIR(1));

		for (;;) {
			switch (tolower(getch())) {
			case 'p':
				d.clean();
				d.is_boss_dead = false;
				pc_t::pc->clean();
				goto game_start;

			case 'q':
				goto end;
			}
		}

		end:

		endwin();

		delete pc_t::pc;
	}
	catch (RlgError err) {
		endwin();

		std::cerr << err.what() << std::endl;
	}

	Logger::del();

	return 0;
}
