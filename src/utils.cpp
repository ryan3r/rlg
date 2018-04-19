// Based on Jeremy's solution for 1.04
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <utils.hpp>
#include <algorithm>
#include <queue>
#include <dungeon.hpp>

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

// count the number of digits in an number
unsigned digit_count(int number) {
	unsigned count = 0;

	if (number == 0) return 1;

	while (number > 0) {
		++count;
		number /= 10;
	}

	return count;
}

// try to find a config file
std::string resolve_config_file(std::string file_name) {
    std::string path = get_default_file(file_name.c_str());
    #ifdef __linux__
    struct stat buf;

    // check if the user overrode this file
    if(!stat(path.c_str(), &buf)) {
        return path;
    }

    return std::string(ETC_CONFIG) + file_name;
    #else
    return path;
    #endif
}
