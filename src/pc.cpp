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
#include <sstream>
#include <logger.hpp>

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

	std::weak_ptr<LoggerMsg> msg_ref;
	LoggerStream(msg_ref) << "Press " << (char) exit_key << " to select a target. Escape or Q to cancel.";

	for (;;) {
		if (should_render) d->render_dungeon();

		should_render = true;

		Logger::clear_messages(Logger::inst()->life_time_key);
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
					LoggerStream(Logger::inst()->life_time_key) << key << " is not a valid command. (press ? for help)";

					should_render = false;
				}
				else {
					uint8_t res = handler(key);
					
					// exit targeting
					if (res == 1) break;

					// unknown key
					if (res == 2) {
						LoggerStream(Logger::inst()->life_time_key) << key << " is not a valid command. (press ? for help)";

						should_render = false;
					}
				}
			}
		}
	}

	// remove the informational message
	if (auto ref = msg_ref.lock()) {
		ref->hide();
		ref->hide();
	}

	teleporting = false;
	d->render_dungeon();
}

void pc_t::next_pos(pair_t &next) {
	bool __is_fogged;
	size_t slot;
	next = position;

	d->render_dungeon();

	Logger::clear_messages(Logger::inst()->life_time_turn);

top:
	Logger::clear_messages(Logger::inst()->life_time_key);
	int key = getch();

	// key sequence to enable cheets
	if (key == 'c') {
		++cheet_level;

		if (cheet_level >= CHEETS_ENABLED) {
			Logger::inst()->life_time_key.push(Logger::inst()->log("Cheets are enabled (type X to disable)"));

			cheet_level = CHEETS_ENABLED;
		}

		goto top;
	}

	// we miss typed reset cheets
	if (cheet_level < CHEETS_ENABLED) {
		cheet_level = 0;
	}

	if (!move_keys(key, next)) {
		switch (key) {
		case 'X':
			cheet_level = 0;
			is_fogged = true;

			d->render_dungeon();
			Logger::inst()->life_time_key.push(Logger::inst()->log("Cheets are disabled"));
			goto top;

		case 'Q':
			deal_damage(get_hp() + 1);
			break;

		case '<':
		case '>':
			if (d->mappair(pc_t::pc->position) != (key == '>' ? terrain_type_t::staircase_down : terrain_type_t::staircase_up)) {
				LoggerStream(Logger::inst()->life_time_key) << "You are not on a" << (key == '>' ? " down" : "n up") << " staircase.";
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
				LoggerStream(Logger::inst()->life_time_key) << "Destroyed " << carry[slot]->name;

				delete carry[slot];
				carry[slot] = nullptr;
			}

			goto top;

		// drop an object in carry slots
		case 'd':
			slot = inventory_prompt("Pick an object to drop", *this, true);
			d->render_dungeon();

			if (slot != NOT_PICKED && carry[slot]) {
				LoggerStream(Logger::inst()->life_time_key) << "Dropped " << carry[slot]->name;

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
					Logger::inst()->life_time_key.push(Logger::inst()->log("No available carry slots please drop or destroy an object"));
					goto top;
				}

				LoggerStream(Logger::inst()->life_time_key) << "Unequipped " << equipment[slot]->name;

				carry[carry_slot] = equipment[slot];
				equipment[slot] = nullptr;

				look_around();
				d->render_dungeon();
			}

			goto top;

		// put on an object
		case 'w':
			slot = inventory_prompt("Pick an object to wear", *this, true);
			d->render_dungeon();

			if (slot != NOT_PICKED && carry[slot]) {
				// can't wear this object
				if (carry[slot]->inventory_type == Object::UNKNOWN) {
					LoggerStream(Logger::inst()->life_time_key) << carry[slot]->name << " can't be worn";

					goto top;
				}

				size_t equip_slot = carry[slot]->inventory_type;

				// use the second ring slot if the first is not empty
				if (carry[slot]->inventory_type == Object::RING && equipment[equip_slot]) {
					++equip_slot;
				}

				std::swap(carry[slot], equipment[equip_slot]);

				LoggerStream(Logger::inst()->life_time_key) << "Putting on " << equipment[equip_slot]->name;
				
				look_around();
				d->render_dungeon();
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
			else if (d->objpair(teleport_target)) {
				display_object(dynamic_cast<Object*>(d->objpair(teleport_target)));
			}

			d->render_dungeon();

			goto top;

		case 'f':
			if (cheet_level < CHEETS_ENABLED) {
				Logger::inst()->life_time_key.push(Logger::inst()->log("Cheets are disabled"));
				goto top;
			}

			is_fogged = !is_fogged;
			d->render_dungeon();
			goto top;

		case 'g':
			if (cheet_level < CHEETS_ENABLED) {
				Logger::inst()->life_time_key.push(Logger::inst()->log("Cheets are disabled"));
				goto top;
			}

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

		// move to the next unread message
		case '=':
			Logger::inst()->scroll(1);
			goto top;

		case '-':
			Logger::inst()->scroll(-1);
			goto top;

		// open the log window
		case 'p':
			Logger::inst()->open_log_window();
			d->render_dungeon();
			goto top;

		default:
			LoggerStream(Logger::inst()->life_time_key) << key << " is not a valid command. (press ? for help)";
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
  int32_t visual_dist = visual_distance();
  pair_t p = position;
  p.y -= visual_dist;

  int32_t startX = p.x - visual_dist;

  if(startX < 0) startX = 0;
  if(p.y < 0) p.y = 0;

  for(; p.y < DUNGEON_Y && p.y < position.y + visual_dist; ++p.y) {
    for(p.x = startX; p.x < DUNGEON_X && p.x < position.x + visual_dist; ++p.x) {
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
	int32_t power = damage.roll();
	int32_t oPower = power;

	// add the power from weapons
	if (equipment[Object::WEAPON]) {
		power += equipment[Object::WEAPON]->damage.roll();
	}
	else if (equipment[Object::OFFHAND]) {
		power += equipment[Object::OFFHAND]->damage.roll();
	}

	// add power from rings
	for (size_t i = Object::RING; i < Object::RING + 2; ++i) {
		if (equipment[i]) {
			power += equipment[i]->damage.roll();
		}
	}

	LoggerStream(Logger::inst()->life_time_turn) << "You attacked " << ((npc_t&)def).name << " it has " << def.get_hp() << " hp left.";

	def.deal_damage(power);

	// check if we killed the boss
	npc_t *monster;
	if (def.get_hp() == 0 && (monster = dynamic_cast<npc_t*>(&def)) && monster->has_attr(npc_t::BOSS)) {
		d->is_boss_dead = true;
	}
}

void pc_t::defend(const character_t &atk) {
	int32_t power = atk.damage.roll();
	int32_t oPower = power;

	// add defense from items
	if (equipment[Object::OFFHAND]) {
		power -= equipment[Object::OFFHAND]->defense.roll();
	}

	if (equipment[Object::ARMOR]) {
		power -= equipment[Object::ARMOR]->defense.roll();
	}

	if (equipment[Object::HELMET]) {
		power -= equipment[Object::HELMET]->defense.roll();
	}

	if (power < 0) power = 0;

	LoggerStream(Logger::inst()->life_time_turn) << "You were attacked by a " << ((npc_t&)atk).name << " it did " << power << " damage.";

	deal_damage(power);
}

// TODO: Doesn't work
int32_t pc_t::get_speed() const {
	int32_t sp = character_t::get_speed();

	for (int i = 0; i < NUM_EQUIPMENT_SLOTS; ++i) {
		if (equipment[i]) {
			sp += equipment[i]->speed;
			sp -= equipment[i]->weight;
		}

		std::clog << equipment[i] << std::endl;
	}

	if (sp < 1) sp = 1;

	return sp;
}

// how far the player can see
int32_t pc_t::visual_distance() {
	int32_t dist = VISUAL_DISTANCE;

	if (equipment[Object::LIGHT]) {
		dist += VISUAL_DISTANCE;
	}

	return dist;
}

// handle dungeon regeneration
void pc_t::regenerate() {
	memset(&map, (int)terrain_type_t::wall, sizeof(map));
	regenerate_dungeon = false;
	teleport_target = position;
}