// Based on Jeremy's solution for 1.04
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <utils.hpp>
#include <algorithm>

#ifdef _WIN32
#undef MOUSE_MOVED
#include <direct.h>

#define mkdir(dir, perm) _mkdir(dir)
#pragma warning(disable:4996)
#endif

int makedirectory(char *dir)
{
  char *slash;

  for (slash = dir + strlen(dir); slash > dir && *slash != '/'; slash--)
    ;

  if (slash == dir) {
    return 0;
  }

  if (mkdir(dir, 0700)) {
    if (errno != ENOENT && errno != EEXIST) {
      fprintf(stderr, "mkdir(%s): %s\n", dir, strerror(errno));
      return 1;
    }
    if (*slash != '/') {
      return 1;
    }
    *slash = '\0';
    if (makedirectory(dir)) {
      *slash = '/';
      return 1;
    }

    *slash = '/';
    if (mkdir(dir, 0700) && errno != EEXIST) {
      fprintf(stderr, "mkdir(%s): %s\n", dir, strerror(errno));
      return 1;
    }
  }

  return 0;
}

int resolve_color(std::string color) {
	// make it case insensitive
	std::transform(color.begin(), color.end(), color.begin(), tolower);

	if (color == "cyan") return 1;
	if (color == "red") return 2;
	if (color == "green") return 3;
	if (color == "black") return 4;
	if (color == "yellow") return 5;
	if (color == "magenta") return 6;
	if (color == "white") return 7;
	if (color == "blue") return 8;

	return 0;
}