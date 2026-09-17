#include "sensor.h"
#include "config.h"

static float        suhuTerakhir  = 0.0;
static bool         valid         = false;
static int          jumlahGagal   = 0;
static unsigned long waktuBaca    = 0;
static const char*  keterangan    = "BELUM_BACA";

// Sensor diasumsikan LM35: output 10 mV per derajat Celsius.
// ESP32 ADC 12 bit (0..4095) dengan referensi 3.3V.
static float adcKeCelsius(int adc) {
  float volt = (adc * 3.3) / 4095.0;
  return volt * 100.0;
}

void sensorInit() {
  pinMode(PIN_SENSOR_SUHU, INPUT);
  jumlahGagal = 0;
  valid = false;
}

void sensorUpdate() {
  // Dibatasi biar ADC nggak dibaca tiap iterasi loop yang jalan ribuan kali per detik.
  if (millis() - waktuBaca < INTERVAL_BACA_SENSOR) return;
  waktuBaca = millis();

  int adc = analogRead(PIN_SENSOR_SUHU);

  // Cek 1: ADC mentok di ujung biasanya bukan suhu beneran, tapi tanda
  // kabel sensor lepas (nilai mentok atas) atau korslet ke ground (mentok bawah).
  if (adc <= 0 || adc >= 4095) {
    valid = false;
    keterangan = "SENSOR_LEPAS";
    if (jumlahGagal < 100) jumlahGagal++;
    return;
  }

  float suhu = adcKeCelsius(adc);

  // Cek 2: hasil konversi harus masih masuk akal secara fisik.
  if (isnan(suhu) || suhu < SUHU_MIN_WAJAR || suhu > SUHU_MAX_WAJAR) {
    valid = false;
    keterangan = "DI_LUAR_BATAS";
    if (jumlahGagal < 100) jumlahGagal++;
    return;
  }

  suhuTerakhir = suhu;
  valid = true;
  keterangan = "OK";
  jumlahGagal = 0;   // satu pembacaan bagus mereset hitungan gagal
}

float sensorSuhu()        { return suhuTerakhir; }
bool  sensorValid()       { return valid; }
int   sensorJumlahGagal() { return jumlahGagal; }
void  sensorResetGagal()  { jumlahGagal = 0; }

bool sensorBermasalah() {
  return jumlahGagal >= MAX_GAGAL_SENSOR;
}

const char* sensorKeterangan() { return keterangan; }