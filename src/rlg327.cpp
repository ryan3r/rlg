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
#include "../include/parser.hpp"

#include <iostream>
#include <fstream>
#include <path.hpp>

const char *victory =
  "\n                                       o\n"
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
  "\n\n\n\n                /\"\"\"\"\"/\"\"\"\"\"\"\".\n"
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
  "            You're dead.  Better luck in the next life.\n\n\n";

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
	std::ofstream log_file("rlg-log.txt");
	std::streambuf *orig_log_rd = std::clog.rdbuf();
	std::clog.rdbuf(log_file.rdbuf());

	try {
		dungeon_t d(
			Parser::parse_file(get_default_file("monster_desc.txt")),
			Parser::parse_file(get_default_file("object_desc.txt"))
		);

		time_t seed;
		int i;
		uint32_t do_load, do_save, do_seed, do_image, do_save_seed,
			do_save_image, do_place_pc;
		uint32_t long_arg;
		char *save_file;
		char *load_file;
		char *pgm_file;
#ifdef __linux__
		struct timeval tv;
#else
		// change the console title
		SetConsoleTitle("rlg327");

		// resize the window
		SMALL_RECT window_size = { 0, 0, 80, 24 };
		SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), 1, &window_size);
#endif

		/* Default behavior: Seed with the time, generate a new dungeon, *
		 * and don't write to disk.                                      */
		do_load = do_save = do_image = do_save_seed =
			do_save_image = do_place_pc = 0;
		do_seed = 1;
		save_file = load_file = NULL;
		d.max_monsters = MAX_MONSTERS;

		/* The project spec requires '--load' and '--save'.  It's common  *
		 * to have short and long forms of most switches (assuming you    *
		 * don't run out of letters).  For now, we've got plenty.  Long   *
		 * forms use whole words and take two dashes.  Short forms use an *
		  * abbreviation after a single dash.  We'll add '--rand' (to     *
		 * specify a random seed), which will take an argument of it's    *
		 * own, and we'll add short forms for all three commands, '-l',   *
		 * '-s', and '-r', respectively.  We're also going to allow an    *
		 * optional argument to load to allow us to load non-default save *
		 * files.  No means to save to non-default locations, however.    *
		 * And the final switch, '--image', allows me to create a dungeon *
		 * from a PGM image, so that I was able to create those more      *
		 * interesting test dungeons for you.                             */

		if (argc > 1) {
			for (i = 1, long_arg = 0; i < argc; i++, long_arg = 0) {
				if (argv[i][0] == '-') { /* All switches start with a dash */
					if (argv[i][1] == '-') {
						argv[i]++;    /* Make the argument have a single dash so we can */
						long_arg = 1; /* handle long and short args at the same place.  */
					}
					switch (argv[i][1]) {
					case 'r':
						if ((!long_arg && argv[i][2]) ||
							(long_arg && strcmp(argv[i], "-rand")) ||
							argc < ++i + 1 /* No more arguments */ ||
							!sscanf(argv[i], "%lu", (unsigned long*)&seed) /* Argument is not an integer */) {
							usage(argv[0]);
						}
						do_seed = 0;
						break;
					case 'n':
						if ((!long_arg && argv[i][2]) ||
							(long_arg && strcmp(argv[i], "-nummon")) ||
							argc < ++i + 1 /* No more arguments */ ||
							!sscanf(argv[i], "%hu", &d.max_monsters)) {
							usage(argv[0]);
						}
						break;
					case 'l':
						if ((!long_arg && argv[i][2]) ||
							(long_arg && strcmp(argv[i], "-load"))) {
							usage(argv[0]);
						}
						do_load = 1;
						if ((argc > i + 1) && argv[i + 1][0] != '-') {
							/* There is another argument, and it's not a switch, so *
							 * we'll treat it as a save file and try to load it.    */
							load_file = argv[++i];
						}
						break;
					case 's':
						if ((!long_arg && argv[i][2]) ||
							(long_arg && strcmp(argv[i], "-save"))) {
							usage(argv[0]);
						}
						do_save = 1;
						if ((argc > i + 1) && argv[i + 1][0] != '-') {
							/* There is another argument, and it's not a switch, so *
							 * we'll save to it.  If it is "seed", we'll save to    *
						 * <the current seed>.rlg327.  If it is "image", we'll  *
						 * save to <the current image>.rlg327.                  */
							if (!strcmp(argv[++i], "seed")) {
								do_save_seed = 1;
								do_save_image = 0;
							}
							else if (!strcmp(argv[i], "image")) {
								do_save_image = 1;
								do_save_seed = 0;
							}
							else {
								save_file = argv[i];
							}
						}
						break;
					case 'i':
						if ((!long_arg && argv[i][2]) ||
							(long_arg && strcmp(argv[i], "-image"))) {
							usage(argv[0]);
						}
						do_image = 1;
						if ((argc > i + 1) && argv[i + 1][0] != '-') {
							/* There is another argument, and it's not a switch, so *
							 * we'll treat it as a save file and try to load it.    */
							pgm_file = argv[++i];
						}
						break;
					case 'p':
						/* PC placement makes no effort to avoid placing *
						 * the PC inside solid rock.                     */
						if ((!long_arg && argv[i][2]) ||
							(long_arg && strcmp(argv[i], "-pc"))) {
							usage(argv[0]);
						}
						if ((d.pc->position.y = atoi(argv[++i])) < 1 ||
							d.pc->position.y > DUNGEON_Y - 2 ||
							(d.pc->position.x = atoi(argv[++i])) < 1 ||
							d.pc->position.x > DUNGEON_X - 2) {
							fprintf(stderr, "Invalid PC position.\n");
							usage(argv[0]);
						}
						do_place_pc = 1;
						break;
					default:
						usage(argv[0]);
					}
				}
				else { /* No dash */
					usage(argv[0]);
				}
			}
		}

		if (do_seed) {
			/* Allows me to generate more than one dungeon *
			 * per second, as opposed to time().           */
#ifdef __linux__
			gettimeofday(&tv, NULL);
			seed = (tv.tv_usec ^ (tv.tv_sec << 20)) & 0xffffffff;
#else
			seed = time(NULL);
#endif
		}

		srand(seed);

		if (do_load) {
			if (load_file) d.read_dungeon(load_file);
			else d.read_dungeon();
		}
		else {
			d.gen_dungeon();
		}

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

		if (should_show_help()) {
			d.pc->look_around();
			d.pc->render_dungeon();
			refresh();
			help();
		}

		while (d.pc->alive && d.has_npcs()) {
			d.pc->look_around();
			d.pc->render_dungeon();
			do_moves(&d);

			if (d.pc->alive) {
				pair_t next;
				pc_t* c = d.pc;

				c->next_pos(next);

				// the dungeon has been regrenerated
				if (c != d.pc) continue;

				move_character(&d, c, next);

				dijkstra(&d);
				dijkstra_tunnel(&d);
			}
		}

		// print the victory/defeat artwork
		erase();

		uint32_t yPos = 0;

		char *msg, *last, *orig;
		orig = msg = last = strdup(d.pc->alive ? victory : tombstone);

		attron(COLOR_PAIR(d.pc->alive ? 5 : 2));

		for (; *msg; ++msg) {
			if (*msg == '\n') {
				*msg = '\0';

				mvprintw(yPos++, 0, "%s", last);

				*msg = '\n';
				last = msg + 1;
			}
		}

		attroff(COLOR_PAIR(d.pc->alive ? 5 : 2));

		free(orig);

		mvprintw(yPos++, 0, "You defended your life in the face of %u deadly beasts.", d.pc->kills_direct);
		mvprintw(yPos++, 0, "You avenged the cruel and untimely murders of %u "
			"peaceful dungeon residents.", d.pc->kills_avenged);

		attron(COLOR_PAIR(1));
		mvprintw(yPos, 0, "[Press ENTER to quit]");
		attroff(COLOR_PAIR(1));

		char key;
		while ((key = getch()) != 10 && key != 13);

		endwin();

		if (do_save) {
			if (do_save_seed) {
				/* 10 bytes for number, please dot, extention and null terminator. */
				save_file = (char*)malloc(18);
				sprintf(save_file, "%ld.rlg327", (unsigned long)seed);
			}
			if (do_save_image) {
				if (!pgm_file) {
					fprintf(stderr, "No image file was loaded.  Using default.\n");
					do_save_image = 0;
				}
				else {
					/* Extension of 3 characters longer than image extension + null. */
					save_file = (char*)malloc(strlen(pgm_file) + 4);
					strcpy(save_file, pgm_file);
					strcpy(strchr(save_file, '.') + 1, "rlg327");
				}
			}
			if (save_file) d.write_dungeon(save_file);
			else d.write_dungeon();

			if (do_save_seed || do_save_image) {
				free(save_file);
			}
		}
	}
	catch (RlgError err) {
		endwin();

		std::cerr << err.what() << std::endl;
	}

	std::clog.rdbuf(orig_log_rd);

	return 0;
}