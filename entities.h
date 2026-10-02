#ifndef _entities_h
#define _entities_h
#include "types.h"

#define create_player(x, y) { \
    create_coords((float)x + 0.5f, (float)y + 0.5f), \
    create_coords(1, 0), \
    create_coords(0, -0.66f), \
    0, 100 }

#define create_enemy(x, y) create_entity(E_ENEMY, x, y, S_STAND, 100)

#define S_STAND   0
#define S_ALERT   1
#define S_MELEE   3
#define S_HIT     4
#define S_DEAD    5
#define S_HIDDEN  6

struct Player {
  Coords pos, dir, plane;
  float   velocity;
  uint8_t health;
};

struct Entity {
  UID     uid;
  Coords  pos;
  uint8_t state;
  uint8_t health;
  uint8_t distance;
  uint8_t timer;
};

Entity create_entity(uint8_t type, uint8_t x, uint8_t y, uint8_t state, uint8_t health);
#endif
