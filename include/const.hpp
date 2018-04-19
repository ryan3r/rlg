#pragma once

#define DUNGEON_X              80
#define DUNGEON_Y              21
#define MIN_ROOMS              5
#define MAX_ROOMS              9
#define ROOM_MIN_X             4
#define ROOM_MIN_Y             2
#define ROOM_MAX_X             14
#define ROOM_MAX_Y             8
#define VISUAL_RANGE           15
#define PC_SPEED               10
#define PC_HP                  1000
#define MAX_MONSTERS           12
#define DUNGEON_SAVE_FILE      "dungeon"
#define DUNGEON_VERSION_FILE   "version"
#define RLG_VERSION            "1.05"
#define DUNGEON_SAVE_SEMANTIC  "RLG327-S2018"
#define ETC_CONFIG             "/etc/rlg327/"
#define DUNGEON_SAVE_VERSION   0U
#define VISUAL_DISTANCE        3
#define CHEETS_ENABLED         3

#ifdef CHEETS_BY_DEFAULT
#define DEFAULT_CHEET_LEVEL CHEETS_ENABLED
#else
#define DEFAULT_CHEET_LEVEL 0
#endif

// TODO: Put this somewhere better
enum class terrain_type_t {
  debug,
  wall,
  wall_immutable,
  floor,
  floor_room,
  floor_hall,
  staircase_up,
  staircase_down
};
