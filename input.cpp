#include <Arduino.h>
#include "input.h"
#include "constants.h"

static int  _vx        = 0;
static int  _vy        = 0;
static bool _fire_edge  = false;
static bool _fire_level = false;
static bool _lastSW    = HIGH;

void input_update() {
  int rawX = analogRead(JOY_X_PIN) - 512;
  int rawY = analogRead(JOY_Y_PIN) - 512;

  // Aplica zona morta individual
  if (abs(rawX) < JOY_DEAD) rawX = 0;
  if (abs(rawY) < JOY_DEAD) rawY = 0;

  // Dominância: se um eixo for significativamente maior que o outro,
  // zera o menor para evitar curvas involuntárias ao andar em linha reta
  if (rawX != 0 && rawY != 0) {
    if (abs(rawY) > abs(rawX) * 2) {
      rawX = 0;   // movendo majoritariamente pra frente/trás → cancela rotação
    } else if (abs(rawX) > abs(rawY) * 2) {
      rawY = 0;   // movendo majoritariamente pros lados → cancela avanço
    }
  }

  _vx = rawX;
  _vy = rawY;

  bool sw     = digitalRead(JOY_SW_PIN);
  _fire_edge  = (_lastSW == HIGH && sw == LOW);
  _fire_level = (sw == LOW);
  _lastSW     = sw;
}

void input_setup() {
  pinMode(JOY_SW_PIN, INPUT_PULLUP);
}

bool input_up()    { return _vy < -JOY_DEAD; }
bool input_down()  { return _vy >  JOY_DEAD; }
bool input_left()  { return _vx < -JOY_DEAD; }
bool input_right() { return _vx >  JOY_DEAD; }
bool input_fire()  { return _fire_edge || _fire_level; }
