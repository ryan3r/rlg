// Based on Jeremy's solution
#pragma once

#include "heap.h"
#include <dims.hpp>
#include <character.hpp>
#include <vector>

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
#define MAX_MONSTERS           12
#define DUNGEON_SAVE_FILE      "dungeon"
#define DUNGEON_VERSION_FILE   "version"
#define RLG_VERSION            "1.05"
#define DUNGEON_SAVE_SEMANTIC  "RLG327-S2018"
#define DUNGEON_SAVE_VERSION   0U

#ifdef __linux__
#define SAVE_DIR               ".rlg327"
#else
#define SAVE_DIR               "rlg327"
#endif

enum class terrain_type_t {
  debug,
  wall,
  wall_immutable,
  floor,
  floor_room,
  floor_hall,
  staircase_up,
  staircase_down,
};

class room_t {
public:
  pair_t position;
  pair_t size;
};

class dungeon_t {
private:
  terrain_type_t map[DUNGEON_Y][DUNGEON_X];
  /* Since hardness is usually not used, it would be expensive to pull it *
   * into cache every time we need a map cell, so we store it in a        *
   * parallel array, rather than using a structure to represent the       *
   * cells.  We may want a cell structure later, but from a performanace  *
   * perspective, it would be a bad idea to ever have the map be part of  *
   * that structure.  Pathfinding will require efficient use of the map,  *
   * and pulling in unnecessary data with each map cell would add a lot   *
   * of overhead to the memory system.                                    */
  uint8_t hardness[DUNGEON_Y][DUNGEON_X];
  character_t *character[DUNGEON_Y][DUNGEON_X];

  int32_t adjacent_to_room(int32_t, int32_t);
  void connect_two_rooms(const room_t*, const room_t*);
  void create_cycle();
  void connect_rooms();
  void smooth_hardness();
  void empty_dungeon();
  void place_rooms();
  void make_rooms();
  void write_rooms(std::ostream&);
  uint32_t calculate_dungeon_size();
  void read_dungeon_map(std::istream&);
  void write_dungeon_map(std::ostream &out);
  void read_rooms(std::istream &in, int num_rooms);
  int calculate_num_rooms(uint32_t);

  void dijkstra_corridor_inv(const pair_t &from, const pair_t &to);
  void dijkstra_corridor(const pair_t &from, const pair_t &to);

  int32_t is_open_space(int32_t y, int32_t x) {
      return !hardnessxy(x, y);
  }

public:
  std::vector<room_t> rooms;
  uint8_t pc_distance[DUNGEON_Y][DUNGEON_X];
  uint8_t pc_tunnel[DUNGEON_Y][DUNGEON_X];
  character_t *pc;
  heap_t events;
  uint16_t num_monsters;
  uint16_t max_monsters;
  uint32_t character_sequence_number;
  /* Game time isn't strictly necessary.  It's implicit in the turn number *
   * of the most recent thing removed from the event queue; however,       *
   * including it here--and keeping it up to date--provides a measure of   *
   * convenience, e.g., the ability to create a new event without explicit *
   * information from the current event.                                   */
  uint32_t time;
  uint32_t is_new;

  dungeon_t();
  ~dungeon_t();

  void gen_dungeon();
  void render_dungeon();
  void write_dungeon(std::string file);
  void write_dungeon();
  void read_dungeon(std::string file);
  void read_dungeon();
  void render_distance_map();
  void render_tunnel_distance_map();
  void place_stairs();
  bool has_npcs();

  terrain_type_t& mappair(const pair_t &pair) { return map[pair.y][pair.x]; }
  terrain_type_t& mapxy(int32_t x, int32_t y) { return map[y][x]; }

  uint8_t& hardnesspair(const pair_t &pair) { return hardness[pair.y][pair.x]; }
  uint8_t& hardnessxy(int32_t x, int32_t y) { return hardness[y][x]; }

  character_t*& charpair(const pair_t &pair) { return character[pair.y][pair.x]; }
  character_t*& charxy(int32_t x, int32_t y) { return character[y][x]; }
};
