#ifndef _constants_h
#define _constants_h

// ── Joystick ──────────────────────────────────────────────────────────────────
// VRx -> A1  (rotação  — eixos calibrados para este hardware)
// VRy -> A0  (movimento frente/trás)
// SW  -> D2  (atirar)
#define JOY_X_PIN     A1
#define JOY_Y_PIN     A0
#define JOY_SW_PIN    2
#define JOY_DEAD      80    // zona morta (±80 em torno de 512)

// ── Buzzer: REMOVIDO ──────────────────────────────────────────────────────────
// SOUND_PIN era 9 — não definimos para desabilitar o som

// ── GFX settings ─────────────────────────────────────────────────────────────
#define OPTIMIZE_SSD1306

#define FRAME_TIME          66.666666
#define ARDUINOJSON_USE_DOUBLE 0
#define RES_DIVIDER         3
#define Z_RES_DIVIDER       4
#define DISTANCE_MULTIPLIER 20
#define MAX_RENDER_DEPTH    12
#define MAX_SPRITE_DEPTH    8

#define ZBUFFER_SIZE        SCREEN_WIDTH / Z_RES_DIVIDER

// Level
#define LEVEL_WIDTH_BASE    3
#define LEVEL_WIDTH         (1 << LEVEL_WIDTH_BASE)
#define LEVEL_HEIGHT        12
#define LEVEL_SIZE          LEVEL_WIDTH / 2 * LEVEL_HEIGHT

// Scenes
#define INTRO               0
#define GAME_PLAY           1

// Game
#define GUN_TARGET_POS      18
#define GUN_SHOT_POS        GUN_TARGET_POS + 4

#define ROT_SPEED           .12
#define MOV_SPEED           .2
#define MOV_SPEED_INV       5

#define JOGGING_SPEED       .005
#define ENEMY_SPEED         .02
#define FIREBALL_SPEED      .2
#define FIREBALL_ANGLES     45

#define MAX_ENTITIES          2
#define MAX_STATIC_ENTITIES   0

#define MAX_ENTITY_DISTANCE   200
#define MAX_ENEMY_VIEW        80
#define ITEM_COLLIDER_DIST    6
#define ENEMY_COLLIDER_DIST   4
#define FIREBALL_COLLIDER_DIST 2
#define ENEMY_MELEE_DIST      6
#define WALL_COLLIDER_DIST    .2

#define ENEMY_MELEE_DAMAGE    8
#define ENEMY_FIREBALL_DAMAGE 20
#define GUN_MAX_DAMAGE        15

// Display
constexpr uint8_t SCREEN_WIDTH     = 128;
constexpr uint8_t SCREEN_HEIGHT    = 64;
constexpr uint8_t HALF_WIDTH       = SCREEN_WIDTH / 2;
constexpr uint8_t RENDER_HEIGHT    = 56;
constexpr uint8_t HALF_HEIGHT      = SCREEN_HEIGHT / 2;

#endif
