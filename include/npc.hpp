// Based on Jeremy's solution for 1.04
#pragma once

#include <stdint.h>
#include <dims.hpp>
#include <character.hpp>
#include <stdlib.h>
#include <utils.hpp>

class dungeon_t;

static uint32_t character_sequence_number = 0;
static const char symbols[] = "0123456789abcdef";

class npc_t: public character_t {
private:
	void next_pos_line_of_sight_tunnel(pair_t &next);
	void next_pos_line_of_sight(pair_t &next);
	void next_pos_rand_tunnel(pair_t &next);
	void next_pos_rand(pair_t &next);
	void next_pos_gradient(pair_t &next);
	void next_pos_00(pair_t &next);
	void next_pos_01(pair_t &next);
	void next_pos_02(pair_t &next);
	void next_pos_03(pair_t &next);
	void next_pos_04(pair_t &next);
	void next_pos_05(pair_t &next);
	void next_pos_06(pair_t &next);
	void next_pos_07(pair_t &next);
	void next_pos_08(pair_t &next);
	void next_pos_09(pair_t &next);
	void next_pos_0a(pair_t &next);
	void next_pos_0b(pair_t &next);
	void next_pos_0c(pair_t &next);
	void next_pos_0d(pair_t &next);
	void next_pos_0e(pair_t &next);
	void next_pos_0f(pair_t &next);

public:
	static constexpr uint8_t SMART = 0;
	static constexpr uint8_t TELEPATH = 1;
	static constexpr uint8_t TUNNEL = 2;
	static constexpr uint8_t ERRATIC = 3;

	int32_t attrs;
	bool have_seen_pc = false;
	pair_t pc_last_known_position;

	npc_t(dungeon_t *_d): character_t(_d, 'x', rand_range(5, 20), ++character_sequence_number), attrs{rand() & 0xf} {
		symbol = symbols[attrs];
	}

	static void gen_monsters(dungeon_t *d);
	virtual void next_pos(pair_t &next);

	bool has_attr(uint8_t index) {
		return attrs & (1 << index);
	}

	virtual ~npc_t() {}
};
