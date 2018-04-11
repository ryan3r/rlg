#ifndef INFO_H
#define INFO_H

#include <dungeon.hpp>
#include <npc.hpp>

constexpr size_t NOT_PICKED = 100000;

void display_monster(npc_t*);
void display_inventory(const std::string &title, pc_t &pc, bool is_carry);
size_t inventory_prompt(const std::string &title, pc_t &pc, bool is_carry);
void list_monsters(dungeon_t *d);
void help();
bool should_show_help();

#endif
