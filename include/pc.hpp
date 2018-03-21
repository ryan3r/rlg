// Based on Jeremy's solution
#pragma once

#include <stdint.h>
#include <dims.hpp>
#include <character.hpp>
#include <const.hpp>

class pc_t: public character_t {
private:
  void place_pc();

public:
  pc_t(dungeon_t *d): character_t(d, '@', PC_SPEED, 0) {}

  virtual bool next_pos(pair_t &dir);
  bool in_room(uint32_t room);
  void config_pc();
};
