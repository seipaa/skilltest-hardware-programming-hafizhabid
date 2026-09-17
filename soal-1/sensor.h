#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>

void  sensorInit();
void  sensorUpdate();          // dipanggil tiap loop, isinya dibatasi pakai millis
float sensorSuhu();            // suhu valid terakhir
bool  sensorValid();           // status pembacaan terakhir
bool  sensorBermasalah();      // true kalau sudah gagal MAX_GAGAL_SENSOR kali
void  sensorResetGagal();
int   sensorJumlahGagal();
const char* sensorKeterangan();

#endif
