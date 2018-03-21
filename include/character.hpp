// Based on Jeremy's solution
#pragma once

#include <stdint.h>
#include <dims.hpp>

class dungeon_t;

class character_t {
protected:
  dungeon_t *d;

public:
  char symbol;
  pair_t position;
  int32_t speed;
  bool alive = true;
  /* Characters use to have a next_turn for the move queue.  Now that it is *
   * an event queue, there's no need for that here.  Instead it's in the    *
   * event.  Similarly, sequence_number was introduced in order to ensure   *
   * that the queue remains stable.  Also no longer necessary here, but in  *
   * this case, we'll keep it, because it provides a bit of interesting     *
   * metadata: locally, how old is this character; and globally, how many   *
   * characters have been created by the game.                              */
  uint32_t sequence_number;
  uint32_t kills_direct = 0;
  uint32_t kills_avenged = 0;

  character_t(dungeon_t *du, char sy, int32_t sp, uint32_t se):
    d{du}, symbol{sy}, speed{sp}, sequence_number{se} {}

  virtual bool next_pos(pair_t &next) = 0;

  uint32_t can_see(const pair_t &target) const;

  uint32_t can_see(const character_t *target) const {
    return can_see(target->position);
  };

  virtual ~character_t() {}
};
