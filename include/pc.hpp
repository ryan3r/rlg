// Based on Jeremy's solution for 1.04
#pragma once

#include <stdint.h>
#include <dims.hpp>
#include <character.hpp>
#include <const.hpp>
#include <string.h>
#include <functional>
#include <object.hpp>

constexpr size_t NUM_EQUIPMENT_SLOTS = 12;
constexpr size_t NUM_CARRY_SLOTS = 10;

class pc_t: public character_t {
private:
  void place_pc();
  bool move_keys(int, pair_t&);
  void target(int, std::function<uint8_t(char)>);

  terrain_type_t map[DUNGEON_Y][DUNGEON_X];

public:
  // fog of war enabled/disabled state
  bool is_fogged = true;
  pair_t teleport_target;
  // teleporting mode
  bool teleporting = false;
  // we want to regenerate
  bool regenerate_dungeon = false;

  pc_t(dungeon_t *d): character_t(d, '@', PC_SPEED, 0, PC_HP, Dice(0, 1, 4)) {
    // initialize the fog of war map
    memset(&map, (int) terrain_type_t::wall, sizeof(map));
	memset(&equipment, 0, sizeof(equipment));
	memset(&carry, 0, sizeof(carry));
  }

  virtual void next_pos(pair_t &dir);
  bool in_room(uint32_t room);
  void config_pc();
  void look_around();

  terrain_type_t& mappair(const pair_t &pair) { return map[pair.y][pair.x]; }

  virtual ~pc_t();

  Object *equipment[NUM_EQUIPMENT_SLOTS];
  Object *carry[NUM_CARRY_SLOTS];
};
