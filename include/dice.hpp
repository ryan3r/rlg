#pragma once
#include <utils.hpp>
#include <stdint.h>

class Dice {
private:
	uint8_t base;
	uint8_t dice;
	uint8_t sides;

public:
	Dice() : Dice(0, 0, 0) {}
	Dice(uint8_t b, uint8_t d, uint8_t s) : base{ b }, dice{ d }, sides{ s } {}

	uint32_t roll() const {
		uint32_t roll = base;

		for (uint32_t rolled = 0; rolled < dice; ++rolled) {
			roll += rand_range(1, sides);
		}

		return roll;
	}
};