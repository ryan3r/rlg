// Based on Jeremy's solution for 1.04
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
#include <algorithm>

pc_t *pc_t::pc = nullptr;

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

// handle keys to move the character/targeting pointer
bool pc_t::move_keys(int key, pair_t &next) {
	switch (key) {
		case '8': case 'k': case KEY_UP:
			if (d->hardnessxy(next.x, next.y - 1) < (teleporting ? 255 : 1))
				--next.y;
			break;

		case '2': case 'j': case KEY_DOWN:
			if (d->hardnessxy(next.x, next.y + 1) < (teleporting ? 255 : 1))
				++next.y;
			break;

		case '1': case 'b':
			if (d->hardnessxy(next.x, next.y + 1) < (teleporting ? 255 : 1))
				++next.y;
			if (d->hardnessxy(next.x - 1, next.y) < (teleporting ? 255 : 1))
				--next.x;
			break;

		case '4': case 'h': case KEY_LEFT:
			if (d->hardnessxy(next.x - 1, next.y) < (teleporting ? 255 : 1))
				--next.x;
			break;

		case '7': case 'y':
			if (d->hardnessxy(next.x - 1, next.y) < (teleporting ? 255 : 1))
				--next.x;
			if (d->hardnessxy(next.x, next.y - 1) < (teleporting ? 255 : 1))
				--next.y;
			break;

		case '6': case 'l': case KEY_RIGHT:
			if (d->hardnessxy(next.x + 1, next.y) < (teleporting ? 255 : 1))
				++next.x;
			break;

		case '3': case 'n':
			if (d->hardnessxy(next.x + 1, next.y) < (teleporting ? 255 : 1))
				++next.x;
			if (d->hardnessxy(next.x, next.y + 1) < (teleporting ? 255 : 1))
				++next.y;
			break;

		case '9': case 'u':
			if (d->hardnessxy(next.x + 1, next.y) < (teleporting ? 255 : 1))
				++next.x;
			if (d->hardnessxy(next.x, next.y - 1) < (teleporting ? 255 : 1))
				--next.y;
			break;

		case '5': case ' ':
			break;

		default:
			return false;
	}

	return true;
}

void pc_t::target(int exit_key, std::function<uint8_t(char)> handler) {
	bool should_render = true;
	teleporting = true;
	teleport_target = position;

	for (;;) {
		if(should_render) d->render_dungeon();
		should_render = true;

		int key = getch();

		// move the target
		if (!move_keys(key, teleport_target)) {
			// finish targeting
			if (key == exit_key) break;
			// cancel targeting
			else if (key == 'Q' || key == 27) {
				teleport_target = position;
				break;
			}
			// pass on the key to the handler
			else {
				if (handler == nullptr) {
					mprintf("%c is not a valid command. (press ? for help)", key);
					should_render = false;
				}
				else {
					uint8_t res = handler(key);
					
					// exit targeting
					if (res == 1) break;

					// unknown key
					if (res == 2) {
						mprintf("%c is not a valid command. (press ? for help)", key);
						should_render = false;
					}
				}
			}
		}
	}

	teleporting = false;
	d->render_dungeon();
}

void pc_t::next_pos(pair_t &next) {
	bool __is_fogged;
	size_t slot;
	next = position;

	top:
	int key = getch();

	if (!move_keys(key, next)) {
		switch (key) {
		case 'Q':
			deal_damage(get_hp());
			break;

		case '<':
		case '>':
			if (d->mappair(pc_t::pc->position) != (key == '>' ? terrain_type_t::staircase_down : terrain_type_t::staircase_up)) {
				mprintf("You are not on a%s staircase.", key == '>' ? " down" : "n up");
				goto top;
			}

			// regenerate the entire dungeon
			regenerate_dungeon = true;
			return;

		case 'm':
			list_monsters(d);
			d->render_dungeon();
			goto top;

		// destroy an object in carry slots
		case 'x':
			slot = inventory_prompt("Pick an object to destroy", *this, true);
			d->render_dungeon();

			if (slot != NOT_PICKED && carry[slot]) {
				mprintf("Destroyed %s", carry[slot]->name.c_str());

				delete carry[slot];
				carry[slot] = nullptr;
			}

			goto top;

		// drop an object in carry slots
		case 'd':
			slot = inventory_prompt("Pick an object to drop", *this, true);
			d->render_dungeon();

			if (slot != NOT_PICKED && carry[slot]) {
				mprintf("Dropped %s", carry[slot]->name.c_str());

				// anything we are standing on gets destroied because we don't implement stacks
				if (d->objpair(position)) {
					delete d->objpair(position);
				}

				d->objpair(position) = carry[slot];
				carry[slot] = nullptr;
			}

			goto top;

		// take off an object
		case 't':
			slot = inventory_prompt("Pick an object to take off", *this, false);
			d->render_dungeon();

			if (slot != NOT_PICKED && equipment[slot]) {
				size_t carry_slot;

				// find an empty carry slot
				for (carry_slot = 0; carry_slot < NUM_CARRY_SLOTS && carry[carry_slot] != nullptr; ++carry_slot);

				if (carry_slot == NUM_CARRY_SLOTS) {
					mprintf("No available carry slots please drop or destroy an object");
					goto top;
				}

				mprintf("Unequipped %s", equipment[slot]->name.c_str());

				carry[carry_slot] = equipment[slot];
				equipment[slot] = nullptr;
			}

			goto top;

		// put on an object
		case 'w':
			slot = inventory_prompt("Pick an object to wear", *this, true);
			d->render_dungeon();

			if (slot != NOT_PICKED && carry[slot]) {
				// can't wear this object
				if (carry[slot]->inventory_type == ObjectType::UNKNOWN) {
					mprintf("%s can't be worn", carry[slot]->name.c_str());
					goto top;
				}

				size_t equip_slot = (size_t) carry[slot]->inventory_type;

				// use the second ring slot if the first is not empty
				if (carry[slot]->inventory_type == ObjectType::RING && equipment[equip_slot]) {
					++equip_slot;
				}

				std::swap(carry[slot], equipment[equip_slot]);

				mprintf("Putting on %s", equipment[equip_slot]->name.c_str());
			}

			goto top;

		// show carry slots
		case 'i': case 'I':
			display_inventory("Inventory", *pc_t::pc, true);
			d->render_dungeon();

			goto top;

		// show equiped slots
		case 'e':
			display_inventory("Equipment", *pc_t::pc, false);
			d->render_dungeon();

			goto top;

		case '?': case '/':
			help();
			d->render_dungeon();
			goto top;

		// get information about a monster
		case 'L':
			target('t', [&](char key) -> uint8_t { return 0; });

			if (d->charpair(teleport_target)) {
				display_monster(dynamic_cast<npc_t*>(d->charpair(teleport_target)));
			}

			d->render_dungeon();

			goto top;

	#ifdef CHEETS
		case 'f':
			is_fogged = !is_fogged;
			d->render_dungeon();
			goto top;

		case 'g':
			d->charpair(position) = nullptr;
			// save the fogged state
			__is_fogged = is_fogged;
			is_fogged = false;

			// target the teleporting location
			target('g', [&](char key) -> uint8_t {
				if (key == 'r') {
					teleport_target.x = rand_range(1, DUNGEON_X - 2);
					teleport_target.y = rand_range(1, DUNGEON_Y - 2);
					return 1;
				}
				else {
					return 2;
				}

				return 0;
			});

			// place the pc
			is_fogged = __is_fogged;
			position = next = teleport_target;

			d->charpair(position) = this;

			if (d->mappair(next) <= terrain_type_t::floor) {
				d->mappair(next) = terrain_type_t::floor_hall;
			}

			look_around();
			d->render_dungeon();
		  
			goto top;
	#endif

		default:
			mprintf("%c is not a valid command. (press ? for help)", key);
			goto top;
		}
	}
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

void pc_t::look_around() {
  pair_t p = position;
  p.y -= VISUAL_DISTANCE;

  int32_t startX = p.x - VISUAL_DISTANCE;

  if(startX < 0) startX = 0;
  if(p.y < 0) p.y = 0;

  for(; p.y < DUNGEON_Y && p.y < position.y + VISUAL_DISTANCE; ++p.y) {
    for(p.x = startX; p.x < DUNGEON_X && p.x < position.x + VISUAL_DISTANCE; ++p.x) {
      if(can_see(p)) {
        mappair(p) = d->mappair(p);
      }
    }
  }
}

pc_t::~pc_t() {
	for (size_t i = 0; i < NUM_EQUIPMENT_SLOTS; ++i) {
		if (equipment[i]) {
			delete equipment[i];
			equipment[i] = nullptr;
		}
	}

	for (size_t i = 0; i < NUM_CARRY_SLOTS; ++i) {
		if (carry[i]) {
			delete carry[i];
			carry[i] = nullptr;
		}
	}
}

void pc_t::attack(character_t &def) const {
	uint32_t power = damage.roll();

	for (int i = 0; i < NUM_EQUIPMENT_SLOTS; ++i) {
		if (equipment[i]) {
			power += equipment[i]->damage.roll();
		}
	}

	def.deal_damage(power);

	// check if we killed the boss
	npc_t *monster;
	if (def.get_hp() == 0 && (monster = dynamic_cast<npc_t*>(&def)) && monster->has_attr(npc_t::BOSS)) {
		d->is_boss_dead = true;
	}
}

void pc_t::defend(const character_t &atk) {
	uint32_t power = atk.damage.roll();

	for (int i = 0; i < NUM_EQUIPMENT_SLOTS; ++i) {
		if (equipment[i]) {
			uint32_t def_roll = equipment[i]->defense.roll();

			power = power > def_roll ? power - def_roll : 0;
		}
	}

	deal_damage(power);
}

// TODO: Doesn't work
int32_t pc_t::get_speed() const {
	int32_t sp = character_t::get_speed();

	for (int i = 0; i < NUM_EQUIPMENT_SLOTS; ++i) {
		if (equipment[i]) {
			sp -= equipment[i]->speed.roll();
		}

		std::clog << equipment[i] << std::endl;
	}

	if (sp < 0) sp = 0;

	return sp;
}

// handle dungeon regeneration
void pc_t::regenerate() {
	memset(&map, (int)terrain_type_t::wall, sizeof(map));
	is_fogged = true;
	regenerate_dungeon = false;
	teleport_target = position;
}