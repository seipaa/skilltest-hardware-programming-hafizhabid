#include "config.h"
#include "sensor.h"
#include "motor.h"
#include "kontrol.h"

void setup() {
  Serial.begin(BAUD_RATE);
  delay(300);   // kasih waktu serial monitor siap, cuma sekali pas boot

  Serial.println();
  Serial.println("=== KONTROL MOTOR - ESP32 ===");

  sensorInit();
  motorInit();
  kontrolInit();
  kontrolCetakBantuan();
}

void loop() {
  // Baca perintah operator kalau ada. Nggak pakai while yang nunggu,
  // jadi loop nggak pernah ketahan di sini.
  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c != '\n' && c != '\r') {
      kontrolPerintah(c);
    }
  }

  kontrolUpdate();
}