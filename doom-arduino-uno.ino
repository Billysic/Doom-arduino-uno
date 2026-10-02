#include "constants.h"
#include "level.h"
#include "sprites.h"
#include "input.h"
#include "entities.h"
#include "types.h"
#include "display.h"
#include "sound.h"

extern void input_update();

#define swap(a, b)  do { typeof(a) temp = a; a = b; b = temp; } while (0)
#define sign(a, b)  (float)(a > b ? 1 : (b > a ? -1 : 0))

// ── Globals ───────────────────────────────────────────────────────────────────
uint8_t scene       = INTRO;
bool    exit_scene  = false;
bool    invert_screen = false;
uint8_t flash_screen  = 0;

Player  player;
Entity  entity[MAX_ENTITIES];
uint8_t num_entities = 0;

// ── Forward declarations ──────────────────────────────────────────────────────
void updateHud();
void renderHud();
void renderStats();
void renderMap(const uint8_t level[], float view_height);
void renderEntities(float view_height);
void renderGun(uint8_t gun_pos, float amount_jogging);
void loopIntro();
void loopGamePlay();

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup(void) {
  setupDisplay();
  input_setup();
  sound_init();
}

void jumpTo(uint8_t target_scene) {
  scene = target_scene;
  exit_scene = true;
}

// ── Level ─────────────────────────────────────────────────────────────────────
void initializeLevel(const uint8_t level[]) {
  for (int8_t y = LEVEL_HEIGHT - 1; y >= 0; y--) {
    for (uint8_t x = 0; x < LEVEL_WIDTH; x++) {
      uint8_t block = getBlockAt(level, x, y);
      if (block == E_PLAYER) {
        player = create_player(x, y);
        return;
      }
    }
  }
}

uint8_t getBlockAt(const uint8_t level[], uint8_t x, uint8_t y) {
  if (x < 0 || x >= LEVEL_WIDTH || y < 0 || y >= LEVEL_HEIGHT) return E_FLOOR;
  return pgm_read_byte(level + (((LEVEL_HEIGHT - 1 - y) * LEVEL_WIDTH + x) / 2))
         >> (!(x % 2) * 4) & 0b1111;
}

bool isSpawned(UID uid) {
  for (uint8_t i = 0; i < num_entities; i++)
    if (entity[i].uid == uid) return true;
  return false;
}

void spawnEntity(uint8_t type, uint8_t x, uint8_t y) {
  if (num_entities >= MAX_ENTITIES) return;
  switch (type) {
    case E_ENEMY:
      entity[num_entities++] = create_enemy(x, y);
      break;
  }
}

void removeEntity(UID uid) {
  uint8_t i = 0;
  bool found = false;
  while (i < num_entities) {
    if (!found && entity[i].uid == uid) { found = true; num_entities--; }
    if (found) entity[i] = entity[i + 1];
    i++;
  }
}

// ── Collision ─────────────────────────────────────────────────────────────────
UID detectCollision(const uint8_t level[], Coords *pos, float rx, float ry, bool only_walls = false) {
  uint8_t bx = int(pos->x + rx);
  uint8_t by = int(pos->y + ry);
  uint8_t block = getBlockAt(level, bx, by);
  if (block == E_WALL) return create_uid(block, bx, by);
  if (only_walls) return UID_null;
  for (uint8_t i = 0; i < num_entities; i++) {
    if (&(entity[i].pos) == pos) continue;
    if (uid_get_type(entity[i].uid) != E_ENEMY) continue;
    if (entity[i].state == S_DEAD || entity[i].state == S_HIDDEN) continue;
    Coords nc = { entity[i].pos.x - rx, entity[i].pos.y - ry };
    uint8_t dist = coords_distance(pos, &nc);
    if (dist < ENEMY_COLLIDER_DIST && dist < entity[i].distance) return entity[i].uid;
  }
  return UID_null;
}

UID updatePosition(const uint8_t level[], Coords *pos, float rx, float ry, bool only_walls = false) {
  UID cx = detectCollision(level, pos, rx, 0, only_walls);
  UID cy = detectCollision(level, pos, 0, ry, only_walls);
  if (!cx) pos->x += rx;
  if (!cy) pos->y += ry;
  return cx || cy ? cx || cy : UID_null;
}

// ── Fire ──────────────────────────────────────────────────────────────────────
void fire() {
  for (uint8_t i = 0; i < num_entities; i++) {
    if (uid_get_type(entity[i].uid) != E_ENEMY) continue;
    if (entity[i].state == S_DEAD || entity[i].state == S_HIDDEN) continue;
    Coords t = { 0, 0 };
    float sx = entity[i].pos.x - player.pos.x;
    float sy = entity[i].pos.y - player.pos.y;
    float inv = 1.0f / (player.plane.x * player.dir.y - player.dir.x * player.plane.y);
    t.x = inv * (player.dir.y * sx - player.dir.x * sy);
    t.y = inv * (-player.plane.y * sx + player.plane.x * sy);
    if (abs(t.x) < 20 && t.y > 0) {
      uint8_t damage = (float)min(GUN_MAX_DAMAGE, GUN_MAX_DAMAGE / (abs(t.x) * entity[i].distance) / 5);
      if (damage > 0) { entity[i].health = max(0, entity[i].health - damage); entity[i].state = S_HIT; entity[i].timer = 4; }
    }
  }
}

// ── Entities update ───────────────────────────────────────────────────────────
void updateEntities(const uint8_t level[]) {
  uint8_t i = 0;
  while (i < num_entities) {
    entity[i].distance = coords_distance(&(player.pos), &(entity[i].pos));
    if (entity[i].timer > 0) entity[i].timer--;
    if (entity[i].distance > MAX_ENTITY_DISTANCE) { removeEntity(entity[i].uid); continue; }
    if (entity[i].state == S_HIDDEN) { i++; continue; }

    if (uid_get_type(entity[i].uid) == E_ENEMY) {
      if (entity[i].health == 0) {
        if (entity[i].state != S_DEAD) { entity[i].state = S_DEAD; entity[i].timer = 6; }
      } else if (entity[i].state == S_HIT) {
        if (entity[i].timer == 0) { entity[i].state = S_ALERT; entity[i].timer = 40; }
      } else {
        if (entity[i].distance > ENEMY_MELEE_DIST && entity[i].distance < MAX_ENEMY_VIEW) {
          if (entity[i].state != S_ALERT) { entity[i].state = S_ALERT; entity[i].timer = 20; }
          else {
            if (entity[i].timer == 0) { entity[i].timer = 20; }
            else {
              updatePosition(level, &(entity[i].pos),
                sign(player.pos.x, entity[i].pos.x) * ENEMY_SPEED * delta,
                sign(player.pos.y, entity[i].pos.y) * ENEMY_SPEED * delta, true);
            }
          }
        } else if (entity[i].distance <= ENEMY_MELEE_DIST) {
          if (entity[i].state != S_MELEE) { entity[i].state = S_MELEE; entity[i].timer = 10; }
          else if (entity[i].timer == 0) {
            player.health = max(0, player.health - ENEMY_MELEE_DAMAGE);
            entity[i].timer = 14;
            flash_screen = 1;
            updateHud();
          }
        } else {
          entity[i].state = S_STAND;
        }
      }
    }
    i++;
  }
}

// ── Raycaster ─────────────────────────────────────────────────────────────────
void renderMap(const uint8_t level[], float view_height) {
  UID last_uid;
  for (uint8_t x = 0; x < SCREEN_WIDTH; x += RES_DIVIDER) {
    float cam_x  = 2 * (float)x / SCREEN_WIDTH - 1;
    float ray_x  = player.dir.x + player.plane.x * cam_x;
    float ray_y  = player.dir.y + player.plane.y * cam_x;
    uint8_t map_x = uint8_t(player.pos.x);
    uint8_t map_y = uint8_t(player.pos.y);
    float dx = abs(1 / ray_x), dy = abs(1 / ray_y);
    int8_t step_x, step_y;
    float side_x, side_y;
    if (ray_x < 0) { step_x = -1; side_x = (player.pos.x - map_x) * dx; }
    else            { step_x =  1; side_x = (map_x + 1.0f - player.pos.x) * dx; }
    if (ray_y < 0) { step_y = -1; side_y = (player.pos.y - map_y) * dy; }
    else            { step_y =  1; side_y = (map_y + 1.0f - player.pos.y) * dy; }

    uint8_t depth = 0; bool hit = false; bool side = false;
    while (!hit && depth < MAX_RENDER_DEPTH) {
      if (side_x < side_y) { side_x += dx; map_x += step_x; side = 0; }
      else                  { side_y += dy; map_y += step_y; side = 1; }
      uint8_t block = getBlockAt(level, map_x, map_y);
      if (block == E_WALL) { hit = true; }
      else if (block == E_ENEMY) {
        Coords mc = { (float)map_x, (float)map_y };
        if (coords_distance(&(player.pos), &mc) < MAX_ENTITY_DISTANCE) {
          UID uid = create_uid(block, map_x, map_y);
          if (last_uid != uid && !isSpawned(uid)) { spawnEntity(block, map_x, map_y); last_uid = uid; }
        }
      }
      depth++;
    }

    if (hit) {
      float distance = (side == 0)
        ? max(1, (map_x - player.pos.x + (1 - step_x) / 2) / ray_x)
        : max(1, (map_y - player.pos.y + (1 - step_y) / 2) / ray_y);
      zbuffer[x / Z_RES_DIVIDER] = min(distance * DISTANCE_MULTIPLIER, 255);
      uint8_t line_height = RENDER_HEIGHT / distance;
      drawVLine(x,
        view_height / distance - line_height / 2 + RENDER_HEIGHT / 2,
        view_height / distance + line_height / 2 + RENDER_HEIGHT / 2,
        GRADIENT_COUNT - int(distance / MAX_RENDER_DEPTH * GRADIENT_COUNT) - side * 2);
    }
  }
}

// ── Sort & translate ──────────────────────────────────────────────────────────
void sortEntities() {
  uint8_t gap = num_entities; bool swapped = false;
  while (gap > 1 || swapped) {
    gap = (gap * 10) / 13;
    if (gap == 9 || gap == 10) gap = 11;
    if (gap < 1) gap = 1;
    swapped = false;
    for (uint8_t i = 0; i < num_entities - gap; i++) {
      uint8_t j = i + gap;
      if (entity[i].distance < entity[j].distance) { swap(entity[i], entity[j]); swapped = true; }
    }
  }
}

Coords translateIntoView(Coords *pos) {
  float sx = pos->x - player.pos.x, sy = pos->y - player.pos.y;
  float inv = 1.0f / (player.plane.x * player.dir.y - player.dir.x * player.plane.y);
  float tx  = inv * (player.dir.y * sx - player.dir.x * sy);
  float ty  = inv * (-player.plane.y * sx + player.plane.x * sy);
  return { tx, ty };
}

// ── Render entities ───────────────────────────────────────────────────────────
void renderEntities(float view_height) {
  sortEntities();
  for (uint8_t i = 0; i < num_entities; i++) {
    if (entity[i].state == S_HIDDEN) continue;
    Coords t = translateIntoView(&(entity[i].pos));
    if (t.y <= 0.1 || t.y > MAX_SPRITE_DEPTH) continue;
    int16_t sx = HALF_WIDTH * (1.0f + t.x / t.y);
    int8_t  sy = RENDER_HEIGHT / 2 + view_height / t.y;
    if (sx < -HALF_WIDTH || sx > SCREEN_WIDTH + HALF_WIDTH) continue;

    if (uid_get_type(entity[i].uid) == E_ENEMY) {
      uint8_t sprite = 0;
      if      (entity[i].state == S_ALERT)  sprite = int(millis() / 500) % 2;
      else if (entity[i].state == S_HIT)    sprite = 3;
      else if (entity[i].state == S_MELEE)  sprite = entity[i].timer > 10 ? 2 : 1;
      else if (entity[i].state == S_DEAD)   sprite = entity[i].timer > 0  ? 3 : 4;
      drawSprite(sx - BMP_IMP_WIDTH * .5f / t.y, sy - 8 / t.y,
        bmp_imp_bits, bmp_imp_mask, BMP_IMP_WIDTH, BMP_IMP_HEIGHT, sprite, t.y);
    }
  }
}

// ── Gun ───────────────────────────────────────────────────────────────────────
void renderGun(uint8_t gun_pos, float amount_jogging) {
  char x = 48;
  char y = RENDER_HEIGHT - gun_pos;
  if (gun_pos > GUN_SHOT_POS - 2)
    display.drawBitmap(x + 6, y - 11, bmp_fire_bits, BMP_FIRE_WIDTH, BMP_FIRE_HEIGHT, 1);
  uint8_t clip = max(0, min(y + BMP_GUN_HEIGHT, RENDER_HEIGHT) - y);
  display.drawBitmap(x, y, bmp_gun_mask, BMP_GUN_WIDTH, clip, 0);
  display.drawBitmap(x, y, bmp_gun_bits,  BMP_GUN_WIDTH, clip, 1);
}

// ── HUD ───────────────────────────────────────────────────────────────────────
void renderHud() {
  drawText(2, 58, F("{}"), 0);
  updateHud();
}

void updateHud() {
  display.clearRect(12, 58, 15, 6);
  drawText(12, 58, player.health);
}

void renderStats() {
  display.clearRect(58, 58, 70, 6);
  drawText(114, 58, int(getActualFps()));
  drawText(82, 58, num_entities);
}

// ── Intro ─────────────────────────────────────────────────────────────────────
void loopIntro() {
  drawText(SCREEN_WIDTH / 2 - 20, 20, F("DOOM-NANO"));
  drawText(SCREEN_WIDTH / 2 - 25, 36, F("PRESS FIRE"));
  display.display();
  while (!exit_scene) {
    input_update();
    if (input_fire()) jumpTo(GAME_PLAY);
  }
}

// ── Gameplay ──────────────────────────────────────────────────────────────────
void loopGamePlay() {
  bool    gun_fired = false;
  uint8_t gun_pos   = 0;
  float   view_height = 0;
  float   jogging     = 0;
  float   rot_speed, old_dir_x, old_plane_x;
  uint8_t fade = GRADIENT_COUNT - 1;

  initializeLevel(sto_level_1);

  do {
    fps();
    input_update();
    memset(display_buf, 0, SCREEN_WIDTH * (RENDER_HEIGHT / 8));

    if (player.health > 0) {
      // Movimento
      if (input_up()) {
        player.velocity += (MOV_SPEED - player.velocity) * .4f;
        jogging = abs(player.velocity) * MOV_SPEED_INV;
      } else if (input_down()) {
        player.velocity += (-MOV_SPEED - player.velocity) * .4f;
        jogging = abs(player.velocity) * MOV_SPEED_INV;
      } else {
        player.velocity *= .5f;
        jogging = abs(player.velocity) * MOV_SPEED_INV;
      }

      // Rotação (cos/sin calculados 1x)
      if (input_right() || input_left()) {
        rot_speed = (input_right() ? -1.0f : 1.0f) * ROT_SPEED * delta;
        float rc = cosf(rot_speed), rs = sinf(rot_speed);
        old_dir_x = player.dir.x;
        player.dir.x   = old_dir_x * rc - player.dir.y * rs;
        player.dir.y   = old_dir_x * rs + player.dir.y * rc;
        old_plane_x = player.plane.x;
        player.plane.x = old_plane_x * rc - player.plane.y * rs;
        player.plane.y = old_plane_x * rs + player.plane.y * rc;
      }

      // Arma
      if      (gun_pos > GUN_TARGET_POS)                    gun_pos -= 1;
      else if (gun_pos < GUN_TARGET_POS)                    gun_pos += 2;
      else if (!gun_fired && input_fire()) { gun_pos = GUN_SHOT_POS; gun_fired = true; fire(); }
      else if (gun_fired && !input_fire()) gun_fired = false;

    } else {
      if (view_height > -10) view_height--;
      else if (input_fire()) jumpTo(INTRO);
      if (gun_pos > 1) gun_pos -= 2;
    }

    // Posição
    if (abs(player.velocity) > 0.003f) {
      updatePosition(sto_level_1, &(player.pos),
        player.dir.x * player.velocity * delta,
        player.dir.y * player.velocity * delta);
    } else {
      player.velocity = 0;
    }

    updateEntities(sto_level_1);
    renderMap(sto_level_1, view_height);
    renderEntities(view_height);
    renderGun(gun_pos, jogging);

    if (fade > 0) {
      fadeScreen(fade);
      fade--;
      if (fade == 0) renderHud();
    } else {
      renderStats();
    }

    if (flash_screen > 0) { invert_screen = !invert_screen; flash_screen--; }
    else if (invert_screen) invert_screen = false;

    display.invertDisplay(false);
    display.display();

    if (input_left() && input_right()) jumpTo(INTRO);

  } while (!exit_scene);
}

// ── Loop principal ────────────────────────────────────────────────────────────
void loop(void) {
  switch (scene) {
    case INTRO:     loopIntro();     break;
    case GAME_PLAY: loopGamePlay();  break;
  }
  for (uint8_t i = 0; i < GRADIENT_COUNT; i++) {
    fadeScreen(i, 0);
    display.display();
    delay(40);
  }
  exit_scene = false;
}
