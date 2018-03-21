// Based on Jeremy's solution
#pragma once

#include <stdint.h>
#include <dims.hpp>
#include <character.hpp>
#include <const.hpp>
#include <string.h>

class pc_t: public character_t {
private:
  void place_pc();

  terrain_type_t map[DUNGEON_Y][DUNGEON_X];
  character_t *character[DUNGEON_Y][DUNGEON_X];

public:
  // fog of war enabled/disabled state
  bool is_fogged = true;
  pair_t teleport_target;
  // teleporting mode
  bool teleporing = false;

  pc_t(dungeon_t *d): character_t(d, '@', PC_SPEED, 0) {
    // initialize the fog of war map
    memset(&map, (int) terrain_type_t::wall, sizeof(map));
    memset(&character, 0, sizeof(character));
  }

  virtual bool next_pos(pair_t &dir);
  bool in_room(uint32_t room);
  void config_pc();
  void render_dungeon();
  void look_around();

  terrain_type_t& mappair(const pair_t &pair) { return map[pair.y][pair.x]; }
  character_t*& charpair(const pair_t &pair) { return character[pair.y][pair.x]; }

  virtual ~pc_t() {}
};
