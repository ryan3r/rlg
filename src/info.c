#include <info.h>
#include <ncurses.h>
#include <utils.h>
#include <stdint.h>
#include <stdlib.h>
#include <dungeon.h>
#include <pc.h>
#include <npc.h>

#define MONSTER_INFO_SIZE 16
#define WINDOW_HEIGHT 22
#define WINDOW_WIDTH 60

char *monster_names[] = {
    "Monster 1",
    "Monster 2",
    "Monster 3",
    "Monster 4",
    "Monster 5",
    "Monster 6",
    "Monster 7",
    "Monster 8",
    "Monster 9",
    "Monster 10",
    "Monster 11",
    "Monster 12",
    "Monster 13",
    "Monster 14",
    "Monster 15",
    "Monster 16",
};

// render the monster list to the window
void print_list(WINDOW *win, dungeon_t *d, char **monster_list, int i) {
    werase(win);
    box(win, 0, 0);

    mvwprintw(win, 0, 22, "| Monster list |");

    for(int j = i; j - i < WINDOW_HEIGHT - 2 && j < d->num_monsters; ++j) {
        mvwprintw(win, j - i + 1, 1, monster_list[j]);
    }

    wrefresh(win);
}

// open the monster list window and enter the monster list key loop
void list_monsters(dungeon_t *d) {
    WINDOW *win = newwin(WINDOW_HEIGHT, WINDOW_WIDTH, (DUNGEON_Y - WINDOW_HEIGHT) / 2, (DUNGEON_X - WINDOW_WIDTH) / 2);
    
    wrefresh(win);

    // get the list of monsters
    char **monster_list = calloc(d->num_monsters, sizeof(character_t));
    uint8_t i = 0;

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

    i = 0;

    print_list(win, d, monster_list, i);

    for(;;) {
        switch(getch()) {
            case 27: case 'Q':
                goto end;

            case KEY_DOWN:
                if(i < d->num_monsters - (WINDOW_HEIGHT - 2)) ++i;
                print_list(win, d, monster_list, i);
                break;
            
            case KEY_UP:
                if(i > 0) --i;
                print_list(win, d, monster_list, i);
                break;
        }
    }

    // release the monster list
    end:
    for(i = 0; i < d->num_monsters; ++i) {
        free(monster_list[i]);
    }

    free(monster_list);

    delwin(win);
}