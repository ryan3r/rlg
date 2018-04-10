// Based on Jeremy's solution for 1.04
#pragma once

#include <stdint.h>
#include <dims.hpp>
#include <character.hpp>
#include <const.hpp>
#include <string.h>
#include <functional>

class pc_t: public character_t {
private:
  void place_pc();
  bool move_keys(char, pair_t&);
  void target(char, std::function<uint8_t(char)>);

  terrain_type_t map[DUNGEON_Y][DUNGEON_X];

public:
  // fog of war enabled/disabled state
  bool is_fogged = true;
  pair_t teleport_target;
  // teleporting mode
  bool teleporting = false;
  // we want to regenerate
  bool regenerate_dungeon = false;

  pc_t(dungeon_t *d): character_t(d, '@', PC_SPEED, 0) {
    // initialize the fog of war map
    memset(&map, (int) terrain_type_t::wall, sizeof(map));
  }

  virtual void next_pos(pair_t &dir);
  bool in_room(uint32_t room);
  void config_pc();
  void look_around();

  terrain_type_t& mappair(const pair_t &pair) { return map[pair.y][pair.x]; }

  virtual ~pc_t() {}
};
