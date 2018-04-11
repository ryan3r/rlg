#include <info.hpp>
#include <utils.hpp>
#include <stdint.h>
#include <stdlib.h>
#include <dungeon.hpp>
#include <pc.hpp>
#include <npc.hpp>
#include <sstream>

#ifdef __linux__
#include <ncurses.h>
#else
#include <curses.h>
#undef MOUSE_MOVED
#include <windows.h>
#endif

#include <fstream>

#define MONSTER_INFO_SIZE 35
#define WINDOW_HEIGHT 22
#define WINDOW_WIDTH 80

// render the monster list to the window
void print_list(WINDOW *win, const std::string &title, const std::vector<std::string> &list, size_t i) {
    wattron(win, COLOR_PAIR(1));

    // remove anything that was already in the window
    werase(win);
    box(win, 0, 0);

    // print the title and list
    mvwprintw(win, 0, (WINDOW_WIDTH - title.size() - 4) / 2, "| %s |", title.c_str());

    wattroff(win, COLOR_PAIR(1));

    for(size_t j = i; j - i < WINDOW_HEIGHT - 3 && j < list.size(); ++j) {
        mvwprintw(win, j - i + 1, 1, list[j].c_str());
    }

    wattron(win, COLOR_PAIR(1));

    // draw a grey bar at the bottom
    for(uint8_t i = 1; i < WINDOW_WIDTH - 1; ++i) {
        mvwaddch(win, WINDOW_HEIGHT - 2, i, ' ');
    }

    // print a help message
    mvwprintw(win, WINDOW_HEIGHT - 2, 1, "Scroll with arrow keys. Q or ESC to close.");

    // print the scroll progress
    if(list.size() > WINDOW_HEIGHT - 3) {
        double progress = (double) i / (list.size() - (WINDOW_HEIGHT - 3));

        if(progress > 1) progress = 1;

        uint8_t iProgress = (uint8_t) (progress * 100);

        uint8_t shift = (iProgress > 9) + (iProgress > 99);

        mvwprintw(win, WINDOW_HEIGHT - 2, WINDOW_WIDTH - 4 - shift, "%d%%", iProgress);
    }

    wattroff(win, COLOR_PAIR(1));

	refresh();
    wrefresh(win);
}

void open_list_window(const std::string &title, const std::vector<std::string> &list, std::function<bool(char)> handler) {
    WINDOW *win = newwin(WINDOW_HEIGHT, WINDOW_WIDTH, (DUNGEON_Y - WINDOW_HEIGHT) / 2, (DUNGEON_X - WINDOW_WIDTH) / 2);

    size_t i = 0;

    // listen for key presses
    for(;;) {
        print_list(win, title, list, i);

		char key;
        switch((key = getch())) {
            case 27: case 'Q':
                goto end;

            case KEY_DOWN:
                if(list.size() > WINDOW_HEIGHT - 3 && i < list.size() - (WINDOW_HEIGHT - 3)) ++i;
                break;

            case KEY_UP:
                if(i > 0) --i;
                break;

			default:
				if (handler && handler(key)) goto end;
        }
    }

    // release the window
    end:
    delwin(win);
}

// open the monster list window and enter the monster list key loop
void list_monsters(dungeon_t *d) {
	std::vector<std::string> monsters;
    size_t i = 0;

    for(pair_t p; p.y < DUNGEON_Y && i < d->num_monsters; ++p.y) {
        for(p.x = 0; p.x < DUNGEON_X && i < d->num_monsters; ++p.x) {
            npc_t *monster = dynamic_cast<npc_t*>(d->charpair(p));

            if(monster != nullptr) {
				++i;

				// check if the pc can even see this monster
				if (
					!(d->pc->can_see(p) &&
						-VISUAL_DISTANCE <= d->pc->position.x - p.x &&
						d->pc->position.x - p.x <= VISUAL_DISTANCE &&
						d->pc->position.y - p.y <= VISUAL_DISTANCE &&
						d->pc->position.y - p.y >= -VISUAL_DISTANCE) &&
					d->pc->is_fogged) {
					continue;
				}

				std::stringstream info;

				info << monster->symbol << ": " << monster->name;

				// calculate the number of spaces to add so the coordinates are right justified
				size_t len = WINDOW_WIDTH - 11 - monster->name.size() - (monster->position.x > 9)
					- (monster->position.y > 9);

				for (size_t i = 0; i < len; ++i) info << " ";

				info << "(" << monster->position.x << ", " << monster->position.y << ")";

				monsters.push_back(info.str());
            }
        }
    }

	if (monsters.empty()) {
		monsters.push_back("No monsters visible");
	}

    open_list_window("Monster list", monsters, nullptr);
}

// convert the pc's inventory to a list
std::vector<std::string> inventory_list(pc_t &pc, bool is_carry) {
	std::vector<std::string> inventory;
	Object **slots = is_carry ? pc.carry : pc.equipment;

	for (size_t i = 0; i < (is_carry ? NUM_CARRY_SLOTS : NUM_EQUIPMENT_SLOTS); ++i) {
		std::stringstream item;

		char id = is_carry ? (i + '0') : (i + 'a');

		item << id << ": " << (slots[i] ? slots[i]->name : "Empty");

		inventory.push_back(item.str());
	}

	return inventory;
}

// prompt the user to pick a slot
size_t inventory_prompt(const std::string &title, pc_t &pc, bool is_carry) {
	size_t choice = NOT_PICKED;

	open_list_window(title, inventory_list(pc, is_carry), [&choice, &is_carry](char key) -> bool {
		if ((is_carry ? '0' : 'a') <= key && key <= (is_carry ? '9' : 'l')) {
			choice = key - (is_carry ? '0' : 'a');
			return true;
		}

		return false;
	});

	return choice;
}

// display information about an object
void display_object(Object *obj) {
	// nothing to display
	if (obj == nullptr) return;

	std::vector<std::string> lines;
	
	size_t i = -1, j = 0;
	while (j < obj->desc.size()) {
		j = obj->desc.find("\n", ++i);

		if (j == std::string::npos) {
			j = obj->desc.size();
		}

		lines.push_back(obj->desc.substr(i, j - i));

		i = j;
	}

	open_list_window(obj->name, lines, nullptr);
}

// display information about a character
void display_monster(npc_t *monster) {
	// nothing to display
	if (monster == nullptr) return;

	std::vector<std::string> lines;

	size_t i = -1, j = 0;
	while (j < monster->desc.size()) {
		j = monster->desc.find("\n", ++i);

		if (j == std::string::npos) {
			j = monster->desc.size();
		}

		lines.push_back(monster->desc.substr(i, j - i));

		i = j;
	}

	open_list_window(monster->name, lines, nullptr);
}

// display the current inventory
void display_inventory(const std::string &title, pc_t &pc, bool is_carry) {
	Object **slots = is_carry ? pc.carry : pc.equipment;

	open_list_window(title, inventory_list(pc, is_carry), [&slots, &is_carry](char key) -> bool {
		// show info for the chosen item
		if ((is_carry ? '0' : 'a') <= key && key <= (is_carry ? '9' : 'l')) {
			size_t choice = key - (is_carry ? '0' : 'a');

			display_object(slots[choice]);
		}

		return false;
	});
}

const std::vector<std::string> help_msg = {
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
    open_list_window("Help", help_msg, nullptr);
}

bool should_show_help() {
    std::string filename = get_default_file(DUNGEON_VERSION_FILE);

    std::ifstream version_file(filename);

    char version[10];

    if(!version_file.fail()) {
        uint8_t i = 0;
        char ch;
        while(i < 9 && version_file) {
            version_file >> ch;
            version[i++] = ch;
        }

        version[i - 1] = '\0';

        version_file.close();

        // check the versions
        if(!strcmp(version, RLG_VERSION)) return false;
    }

    // write the new version
    strncpy(version, RLG_VERSION, sizeof(RLG_VERSION));

    std::ofstream vfile_out(filename);

    if(!vfile_out.fail()) {
        vfile_out << version;
        vfile_out.close();
    }

    return true;
}
