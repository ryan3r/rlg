#include <info.hpp>
#include <utils.hpp>
#include <stdint.h>
#include <stdlib.h>
#include <dungeon.hpp>
#include <pc.hpp>
#include <npc.hpp>
#include <sstream>
#include <map>

#ifdef __linux__
#include <ncurses.h>
#else
#include <curses.h>
#undef MOUSE_MOVED
#include <windows.h>
#endif

#include <fstream>

#ifdef _WIN32
#define strncpy strncpy_s
#endif

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
    mvwprintw(win, 0, (WINDOW_WIDTH - (int) title.size() - 4) / 2, "| %s |", title.c_str());

    wattroff(win, COLOR_PAIR(1));

    for(size_t j = i; j - i < WINDOW_HEIGHT - 3 && j < list.size(); ++j) {
        mvwprintw(win, (int) (j - i + 1), 1, list[j].c_str());
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
	open_list_window(title, list, handler, false);
}

void open_list_window(const std::string &title, const std::vector<std::string> &list, std::function<bool(char)> handler, bool start_at_end) {
    WINDOW *win = newwin(WINDOW_HEIGHT, WINDOW_WIDTH, (DUNGEON_Y - WINDOW_HEIGHT) / 2, (DUNGEON_X - WINDOW_WIDTH) / 2);

    size_t i = start_at_end ? list.size() - (WINDOW_HEIGHT - 3) : 0;

    // listen for key presses
    for(;;) {
        print_list(win, title, list, i);

		int key;
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
	std::map<char, npc_t*> monsters_by_symbol;
    size_t i = 0;

    for(pair_t p; p.y < DUNGEON_Y && i < d->num_monsters; ++p.y) {
        for(p.x = 0; p.x < DUNGEON_X && i < d->num_monsters; ++p.x) {
            npc_t *monster = dynamic_cast<npc_t*>(d->charpair(p));

            if(monster != nullptr) {
				++i;

				// check if the pc can even see this monster
				if (
					!(pc_t::pc->can_see(p) &&
						-pc_t::pc->visual_distance() <= pc_t::pc->position.x - p.x &&
						pc_t::pc->position.x - p.x <= pc_t::pc->visual_distance() &&
						pc_t::pc->position.y - p.y <= pc_t::pc->visual_distance() &&
						pc_t::pc->position.y - p.y >= -pc_t::pc->visual_distance()) &&
					pc_t::pc->is_fogged) {
					continue;
				}

				std::stringstream info;

				info << monster->symbol << ": " << monster->name;

				// calculate the number of spaces to add so the coordinates are right justified
				size_t len = WINDOW_WIDTH - 11 - monster->name.size() - (monster->position.x > 9)
					- (monster->position.y > 9);

				for (size_t i = 0; i < len; ++i) info << " ";

				info << "(" << monster->position.x << ", " << monster->position.y << ")";

				monsters_by_symbol[monster->symbol] = monster;
				monsters.push_back(info.str());
            }
        }
    }

	if (monsters.empty()) {
		monsters.push_back("No monsters visible");
	}

	open_list_window("Monster list", monsters, [&monsters_by_symbol](char key) -> bool {
		if (('a' <= key && key <= 'z') || ('A' <= key && key <= 'Z')) {
			auto found = monsters_by_symbol.find(key);

			if (found != monsters_by_symbol.end()) {
				display_monster(found->second);
			}
		}

		return false;
	});
}

const std::string SLOT_NAMES[] = {
	"Weapon",
	"Offhand",
	"Ranged",
	"Armor",
	"Helmet",
	"Cloak",
	"Gloves",
	"Boots",
	"Amulet",
	"Light",
	"Ring",
	"Ring"
};

// convert the pc's inventory to a list
std::vector<std::string> inventory_list(pc_t &pc, bool is_carry) {
	std::vector<std::string> inventory;
	Object **slots = is_carry ? pc.carry : pc.equipment;

	for (size_t i = 0; i < (is_carry ? NUM_CARRY_SLOTS : NUM_EQUIPMENT_SLOTS); ++i) {
		std::stringstream item;

		char id = (char) (is_carry ? (i + '0') : (i + 'a'));

		item << id << ": " << (slots[i] ? slots[i]->name : "");

		if (is_carry && slots[i]) {
			item << " [" << slots[i]->type << "]";
		}
		else if(!is_carry) {
			size_t pad = WINDOW_WIDTH - 4 - item.str().size() - SLOT_NAMES[i].size();

			while (pad-- > 0) item << " ";

			item << "[" << SLOT_NAMES[i] << "]";
		}

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

void add_description_lines(std::vector<std::string> &lines, std::string desc) {
	size_t i = -1, j = 0;
	while (j < desc.size()) {
		j = desc.find("\n", ++i);

		if (j == std::string::npos) {
			j = desc.size();
		}

		lines.push_back(desc.substr(i, j - i));

		i = j;
	}
}

// display information about an object
void display_object(Object *obj) {
	// nothing to display
	if (obj == nullptr) return;

	std::vector<std::string> lines;

	std::stringstream stat;

	stat << "Speed: " << obj->speed;
	lines.push_back(stat.str());

	stat.clear();
	stat.str(std::string());
	stat << "Weight: " << obj->weight;
	lines.push_back(stat.str());

	stat.clear();
	stat.str(std::string());
	stat << "Attack: " << obj->damage;
	lines.push_back(stat.str());

	stat.clear();
	stat.str(std::string());
	stat << "Defense: " << obj->defense;
	lines.push_back(stat.str());

	lines.push_back("");

	add_description_lines(lines, obj->desc);

	open_list_window(obj->name, lines, nullptr);
}

// display information about a character
void display_monster(npc_t *monster) {
	// nothing to display
	if (monster == nullptr) return;

	std::vector<std::string> lines;

	std::stringstream stat;

	stat << "Speed: " << monster->get_speed();
	lines.push_back(stat.str());

	stat.clear();
	stat.str(std::string());
	stat << "Attack: " << monster->damage;
	lines.push_back(stat.str());

	stat.clear();
	stat.str(std::string());
	stat << "Hp: " << monster->get_hp();
	lines.push_back(stat.str());

	lines.push_back("");

	add_description_lines(lines, monster->desc);

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
    "m           | Display a list of monsters",
    "w           | Wear item",
    "i           | Display inventory",
    "t           | Take off item",
    "x           | Destroy an item",
    "d           | Drop an item",
    "e           | Display equipped items",
    "L           | Get info about a monster or item"
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
