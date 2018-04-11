// Based on Jeremy's solution for 1.04
#pragma once

#include <stdint.h>
#include <dims.hpp>
#include <dice.hpp>

class dungeon_t;

class character_t {
private:
	uint32_t hp;
	int32_t speed;

protected:
  dungeon_t *d;

public:
  Dice damage;
  char symbol;
  pair_t position;
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

  character_t(dungeon_t *du, char sy, int32_t sp, uint32_t se, uint32_t h, Dice d):
	  d{ du }, symbol{ sy }, speed{ sp }, sequence_number{ se }, hp{ h }, damage{ d } {}

  virtual void next_pos(pair_t &next) = 0;

  uint32_t can_see(const pair_t &target) const;

  uint32_t can_see(const character_t *target) const {
    return can_see(target->position);
  };

  // deal damage to this character
  void deal_damage(uint32_t dmg) {
	  hp = dmg > hp ? 0 : hp - dmg;
  }

  bool alive() const { return hp > 0; }

  virtual void attack(character_t&) const;
  virtual int32_t get_speed() const;

  virtual ~character_t() {}

  uint32_t get_hp() { return hp; }
};
