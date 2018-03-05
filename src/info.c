#include <info.h>
#include <ncurses.h>
#include <utils.h>
#include <stdint.h>
#include <stdlib.h>
#include <dungeon.h>
#include <pc.h>
#include <npc.h>

#define MONSTER_INFO_SIZE 14
#define WINDOW_HEIGHT 22
#define WINDOW_WIDTH 60

void print_list(WINDOW *win, dungeon_t *d, char **monster_list, int i) {
    werase(win);

    for(int j = i; j - i < WINDOW_HEIGHT - 2 && j < d->num_monsters; ++j) {
        mvwprintw(win, j - i, 0, monster_list[j]);
    }

    wrefresh(win);
}

void list_monsters(dungeon_t *d) {
    WINDOW *win = newwin(WINDOW_HEIGHT, WINDOW_WIDTH, 1, 10);
    box(win, 0, 0);
    
    wrefresh(win);

    // create a subwindow inside the border so we don't overwrite the border
    WINDOW *inner_win = derwin(win, WINDOW_HEIGHT - 2, WINDOW_WIDTH - 2, 1, 1);

    // get the list of monsters
    char decoder[] = "0123456789abcdef";
    char **monster_list = calloc(d->num_monsters, sizeof(character_t));
    uint8_t i = 0;

    for(uint8_t y = 0; y < DUNGEON_Y && i < d->num_monsters; ++y) {
        for(uint8_t x = 0; x < DUNGEON_X && i < d->num_monsters; ++x) {
            character_t *monster = d->character[y][x];

            if(monster != NULL && monster->pc == NULL) {
                char *monster_info = malloc(MONSTER_INFO_SIZE * sizeof(char));

                snprintf(monster_info, MONSTER_INFO_SIZE, "%c at (%d, %d)",
                    decoder[monster->npc->characteristics], monster->position[dim_x], monster->position[dim_y]);
                
                monster_list[i++] = monster_info;
            }
        }
    }

    i = 0;

    print_list(inner_win, d, monster_list, i);

    for(;;) {
        switch(getch()) {
            case 27: case 'Q':
                goto end;

            case KEY_DOWN:
                if(i < d->num_monsters - 3 - WINDOW_HEIGHT) ++i;
                print_list(inner_win, d, monster_list, i);
                break;
            
            case KEY_UP:
                if(i > 0) --i;
                print_list(inner_win, d, monster_list, i);
                break;
        }
    }

    // release the monster list
    end:
    for(i = 0; i < d->num_monsters; ++i) {
        free(monster_list[i]);
    }

    free(monster_list);

    delwin(inner_win);
    delwin(win);
}