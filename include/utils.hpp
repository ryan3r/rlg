// Based on Jeremy's solution
#pragma once

#ifdef __linux__
#include <ncurses.h>
#else
#include <curses.h>
#endif

/* Returns true if random float in [0,1] is less than *
 * numerator/denominator.  Uses only integer math.    */
# define rand_under(numerator, denominator) \
  (rand() < ((RAND_MAX / denominator) * numerator))

/* Returns random integer in [min, max]. */
# define rand_range(min, max) ((rand() % (((max) + 1) - (min))) + (min))

int makedirectory(char *dir);

#define mprintf(...) {           \
  move(0, 0);                    \
  clrtoeol();                    \
  mvprintw(0, 0, __VA_ARGS__);   \
}

template<typename T> T min(T a, T b) {
  return a < b ? a : b;
}
