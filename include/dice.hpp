#pragma once
#include <utils.hpp>
#include <stdint.h>
#include <ostream>

class Dice {
private:
	int32_t base;
	int32_t dice;
	int32_t sides;

public:
	Dice() : Dice(0, 0, 0) {}
	Dice(int32_t b, int32_t d, int32_t s) : base{ b }, dice{ d }, sides{ s } {}

	int32_t roll() const {
		int32_t roll = base;

		for (int32_t rolled = 0; rolled < dice; ++rolled) {
			roll += rand_range(1, sides);
		}

		return roll;
	}

	friend std::ostream& operator<<(std::ostream&, const Dice&);
};

inline std::ostream& operator<<(std::ostream &out, const Dice &die) {
	if (die.dice > 0) {
		out << (int) die.base << " + " << (int) die.dice << " roll" << (die.dice == 1 ? "" : "s")
			<< " of a " << (int) die.sides << " sided " << (die.dice == 1 ? "die" : "dice");
	}
	else {
		out << (int) die.base;
	}

	return out;
}
