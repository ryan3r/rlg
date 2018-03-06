#include <info.h>
#include <utils.h>
#include <stdint.h>
#include <stdlib.h>
#include <dungeon.h>
#include <pc.h>
#include <npc.h>

#ifdef __linux__
#include <ncurses.h>
#else
#include <curses.h>
#endif

#define MONSTER_INFO_SIZE 16
#define WINDOW_HEIGHT 22
#define WINDOW_WIDTH 60

char *monster_names[] = {
    "Troll",
    "Goblin",
    "Pixie",
    "Wolf",
    "Gnome",
    "Phoenix",
    "Satyr",
    "Centaur",
    "Dragon",
    "Angry troll",
    "Hungry wolf",
    "Hungry gnome",
    "Angry phoenix",
    "Sleepy floof",
    "Angry dragon",
    "Hungry wolf"
};

// render the monster list to the window
void print_list(WINDOW *win, char *title, char **list, size_t i, size_t length) {
    wattron(win, COLOR_PAIR(1));

    // remove anything that was already in the window
    werase(win);
    box(win, 0, 0);

    // print the title and list
    mvwprintw(win, 0, (WINDOW_WIDTH - strlen(title) - 4) / 2, "| %s |", title);

    wattroff(win, COLOR_PAIR(1));

    for(size_t j = i; j - i < WINDOW_HEIGHT - 3 && j < length; ++j) {
        mvwprintw(win, j - i + 1, 1, list[j]);
    }

    wattron(win, COLOR_PAIR(1));

    // draw a grey bar at the bottom
    for(uint8_t i = 1; i < WINDOW_WIDTH - 1; ++i) {
        mvwaddch(win, WINDOW_HEIGHT - 2, i, ' ');
    }

    // print a help message
    mvwprintw(win, WINDOW_HEIGHT - 2, 1, "Scroll with arrow keys. Q or ESC to close.");

    // print the scroll progress
    if(length > WINDOW_HEIGHT - 3) {
        double progress = (double) i / (length - (WINDOW_HEIGHT - 3));

        if(progress > 1) progress = 1;

        uint8_t iProgress = (uint8_t) (progress * 100);

        uint8_t shift = (iProgress > 9) + (iProgress > 99);

        mvwprintw(win, WINDOW_HEIGHT - 2, WINDOW_WIDTH - 4 - shift, "%d%%", iProgress);
    }

    wattroff(win, COLOR_PAIR(1));

    wrefresh(win);
}

void open_list_window(char *title, char **list, size_t length) {
    WINDOW *win = newwin(WINDOW_HEIGHT, WINDOW_WIDTH, (DUNGEON_Y - WINDOW_HEIGHT) / 2, (DUNGEON_X - WINDOW_WIDTH) / 2);

    size_t i = 0;

    // listen for key presses
    for(;;) {
        print_list(win, title, list, i, length);

        switch(getch()) {
            case 27: case 'Q':
                goto end;

            case KEY_DOWN:
                if(length > WINDOW_HEIGHT - 3 && i < length - (WINDOW_HEIGHT - 3)) ++i;
                break;
            
            case KEY_UP:
                if(i > 0) --i;
                break;
        }
    }

    // release the window
    end:
    delwin(win);
}

// open the monster list window and enter the monster list key loop
void list_monsters(dungeon_t *d) {
    // get the list of monsters
    char **monster_list = calloc(d->num_monsters, sizeof(character_t));
    size_t i = 0;

    for(uint8_t y = 0; y < DUNGEON_Y && i < d->num_monsters; ++y) {
        for(uint8_t x = 0; x < DUNGEON_X && i < d->num_monsters; ++x) {
            character_t *monster = d->character[y][x];

            if(monster != NULL && monster->pc == NULL) {
                char *name = monster_names[monster->npc->characteristics];

                size_t size = MONSTER_INFO_SIZE + strlen(name);
                char *monster_info = malloc(size * sizeof(char));

                snprintf(monster_info, size, "%c: %s at (%d, %d)", monster->symbol, name, monster->position[dim_x], monster->position[dim_y]);
                
                monster_list[i++] = monster_info;
            }
        }
    }

    open_list_window("Monster list", monster_list, d->num_monsters);
    
    for(i = 0; i < d->num_monsters; ++i) {
        free(monster_list[i]);
    }

    free(monster_list);
}

char *help_msg[] = {
    "Welcome adventurer",
    "==========================================================",
    "You find your self in a dungeon with no clue how you got",
    "there. But there's no time to figure it out because the",
    "dungeon is full of monsters.  Start by exploring the",
    "dungeon while avoiding monsters.",
    "",
    "Game",
    "==========================================================",
    "Monsters are represented by red letters and numbers.",
    "You are the cyan @ symbol.",
    "The . symbols are rooms and the # symbols are corridors.",
    "Finally the < and > are staircases.",
    "",

    "The game is turn based so each player takes a turn.  Be",
    "aware that monsters can move multiple times if they are",
    "faster than you.",
    "",
    "Controls",
    "==========================================================",
    "7 or y      | Move up and to the left",
    "8 or k or w | Move up",
    "9 or u      | Move up and to the right",
    "6 or l or d | Move to the right",
    "3 or n      | Move down and to the right",
    "2 or j or s | Move down",
    "1 or b      | Move down and to the left",
    "4 or h or a | Move to the left",
    ">           | Go down stairs (only >)",
    "<           | Go down stairs (only <)",
    "5 or space  | Skip turn",
    "m           | Display a list of monsters"
};

// print the README
void help() {
    open_list_window("Help", help_msg, sizeof(help_msg) / sizeof(*help_msg));
}