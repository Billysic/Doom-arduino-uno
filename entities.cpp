#include <stdint.h>
#include "entities.h"
#include "types.h"
#include "constants.h"

Entity create_entity(uint8_t type, uint8_t x, uint8_t y, uint8_t initialState, uint8_t initialHealth) {
  UID uid = create_uid(type, x, y);
  Coords pos = create_coords((float)x + .5f, (float)y + .5f);
  Entity e = { uid, pos, initialState, initialHealth, 0, 0 };
  return e;
}
