// Based on Jeremy's solution for 1.04
#pragma once

#include <stdint.h>
#include <ostream>

// NOTE: Renamed to avoid collisions with std::pair
class pair_t {
public:
	int32_t x;
	int32_t y;

	pair_t(): pair_t(0, 0) {}
	pair_t(int32_t _x, int32_t _y): x{_x}, y{_y} {}

	bool operator ==(const pair_t &p) const { return x == p.x && y == p.y; }
	bool operator !=(const pair_t &p) const { return !(*this == p); }

	pair_t operator +(const pair_t &p) const { return pair_t(x + p.x, y + p.y); }
	pair_t operator -(const pair_t &p) const { return pair_t(x - p.x, y - p.y); }

	pair_t& operator +=(const pair_t &p) { x += p.x; y += p.y; return *this; }
	pair_t& operator -=(const pair_t &p) { x -= p.x; y -= p.y; return *this; }

	bool operator <(const pair_t &p) const {
		if(x == p.x) return y < p.y;

		return x < p.x;
	}

	bool operator >(const pair_t &p) const { return !((*this == p) || (*this < p)); }
};

inline std::ostream& operator <<(std::ostream &out, const pair_t &p) {
	out << "(" << p.x << ", " << p.y << ")";
	return out;
}
