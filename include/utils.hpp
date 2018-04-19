// Based on Jeremy's solution for 1.04
#pragma once

#ifdef __linux__
#include <ncurses.h>
#else
#include <curses.h>
#endif

#include <string>
#include <string.h>

/* Returns true if random float in [0,1] is less than *
 * numerator/denominator.  Uses only integer math.    */
#define rand_under(numerator, denominator) \
  (rand() < ((RAND_MAX / denominator) * numerator))

/* Returns random integer in [min, max]. */
#define rand_range(min, max) ((rand() % (((max) + 1) - (min))) + (min))

int makedirectory(char *dir);

int resolve_color(std::string);

class RlgError : public std::exception {
private:
	std::string msg;

public:
	RlgError(const std::string s) : msg{ s } {}

	const char* what() const noexcept {
#ifdef _WIN32
		return _strdup(msg.c_str());
#else
		return strdup(msg.c_str());
#endif
	}
};

unsigned digit_count(int number);

std::string resolve_config_file(std::string);
