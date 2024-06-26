/*
 * DRV8833ESCServo - A servo-like library for the DRV8833ESC dual motor driver.
 */

#include "Arduino.h"
#include "DRV8833ESC.h"

DRV8833ESC::DRV8833ESC()
{
}

void DRV8833ESC::attach(int pin1, int pin2, int deadzone)
{
  if (!this->initialized)
  {
    this->pin1 = pin1;
    this->pin2 = pin2;
    this->deadzone = deadzone;

    pinMode(pin1, OUTPUT);
    pinMode(pin2, OUTPUT);
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, LOW);

    this->initialized = true;
    this->break_enabled = false;
  }
}

void DRV8833ESC::write(int value)
{
  int speed = map(min(90, abs(value-90)),
                       0, 90, 0, PWMRANGE);
  
  if (value>=90-deadzone && value<=90+deadzone)
  {
    digitalWrite(this->pin1, this->break_enabled ? HIGH : LOW);
    digitalWrite(this->pin2, this->break_enabled ? HIGH : LOW);
  }
  else
  if (value<90)
  {
    if (value<0)
    {
      // Set to full reverse
      digitalWrite(this->pin1, LOW);
      digitalWrite(this->pin2, HIGH);
    }
    else
    {
      digitalWrite(this->pin1, LOW);
      analogWrite(this->pin2, speed);
    }
  }
  else
  if (value>90)
  {
    if (value>180)
    {
      // Set to full forward
      digitalWrite(this->pin1, HIGH);
      digitalWrite(this->pin2, LOW);
    }
    else
    {
      analogWrite(this->pin1, speed);
      digitalWrite(this->pin2, LOW);
    }
  }
}

void DRV8833ESC::set_break(bool enabled)
{
  this->break_enabled = enabled;
}
