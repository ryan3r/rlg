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
  void destroy_object();
  void drop_object();
  void take_off_object();
  void wear_object();

  terrain_type_t map[DUNGEON_Y][DUNGEON_X];

public:
	static pc_t *pc;

  // fog of war enabled/disabled state
  bool is_fogged = true;
  // teleport/target mode
  bool teleporting = false;
  pair_t teleport_target;
  // we want to regenerate
  bool regenerate_dungeon = false;
  // do we have cheets enabled
  uint8_t cheet_level = DEFAULT_CHEET_LEVEL;

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
  void regenerate();

  Object *equipment[NUM_EQUIPMENT_SLOTS];
  Object *carry[NUM_CARRY_SLOTS];

  virtual void attack(character_t&) const;
  void defend(const character_t &atk);

  virtual int32_t get_speed() const;

  int32_t visual_distance();
};