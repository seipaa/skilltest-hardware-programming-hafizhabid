#ifndef KONTROL_H
#define KONTROL_H

#include <Arduino.h>

void kontrolInit();
void kontrolUpdate();            // state machine, dipanggil tiap loop
void kontrolPerintah(char c);    // perintah dari operator lewat serial
void kontrolCetakStatus();
void kontrolCetakBantuan();

#endif