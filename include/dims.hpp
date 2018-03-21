// Based on Jeremy's solution for 1.04
#pragma once

#include <const.hpp>
#include <stdint.h>
#include <ostream>

// NOTE: Renamed to avoid collisions with std::pair
class pair_t {
public:
	int32_t x;
	int32_t y;

	pair_t(): pair_t(0, 0) {}
	pair_t(int32_t _x, int32_t _y): x{_x}, y{_y} {
		force_inbounds();
	}

	bool operator ==(const pair_t &p) const { return x == p.x && y == p.y; }
	bool operator !=(const pair_t &p) const { return !(*this == p); }

	pair_t operator +(const pair_t &p) const { return pair_t(x + p.x, y + p.y); }
	pair_t operator -(const pair_t &p) const { return pair_t(x - p.x, y - p.y); }

	pair_t& operator +=(const pair_t &p) { x += p.x; y += p.y; force_inbounds(); return *this; }
	pair_t& operator -=(const pair_t &p) { x -= p.x; y -= p.y; force_inbounds(); return *this; }

	bool operator <(const pair_t &p) const {
		if(x == p.x) return y < p.y;

		return x < p.x;
	}

	bool operator >(const pair_t &p) const { return !((*this == p) || (*this < p)); }

	void force_inbounds() {
		if(x < 0) x = 0;
		if(y < 0) y = 0;
		if(x >= DUNGEON_X) x = DUNGEON_X - 1;
		if(y >= DUNGEON_Y) y = DUNGEON_Y - 1;
	}
};

inline std::ostream& operator <<(std::ostream &out, const pair_t &p) {
	out << "(" << p.x << ", " << p.y << ")";
	return out;
}
