/*
* DRV8833ESCServo - A servo-like library for the DRV8833ESC dual motor driver.
 */

#ifndef DRV8833ESC_H
#define DRV8833ESC_H

#include "Arduino.h"

class DRV8833ESC
{
public:
  DRV8833ESC();
  void attach(int pin1, int pin2, int deadzone);
  // Range 0 to 180 (as with servo). With 90 being neutral.
  void write(int value);
  void set_break(bool enabled);

private:
  bool initialized = false;
  bool break_enabled = false;
  int pin1 = -1;
  int pin2 = -1;
  int deadzone = 0;
};

#endif // DRV8833ESC_H
