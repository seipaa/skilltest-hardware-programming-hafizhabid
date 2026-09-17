#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>

void motorInit();
void motorSetPwm(int pwm);   // nilai di luar 0..255 otomatis dipotong
void motorStop();
int  motorPwmSekarang();

#endif