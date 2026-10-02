#ifndef _types_h
#define _types_h

#define UID_null 0
#define E_FLOOR   0x0
#define E_WALL    0xF
#define E_PLAYER  0x1
#define E_ENEMY   0x2

typedef uint16_t UID;
typedef uint8_t  EType;

struct Coords { float x; float y; };

UID     create_uid(EType type, uint8_t x, uint8_t y);
EType   uid_get_type(UID uid);
Coords  create_coords(float x, float y);
uint8_t coords_distance(Coords* a, Coords* b);
#endif
