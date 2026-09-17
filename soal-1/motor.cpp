#include "kontrol.h"
#include "config.h"
#include "sensor.h"
#include "motor.h"

// State sistem. Dipisah dari mode karena satu mode bisa punya
// beberapa kondisi (misal mode AUTO bisa lagi IDLE atau lagi jalan).
enum State { IDLE, AUTO_RUN, MANUAL_RUN, FAULT };
enum Mode  { AUTO, MANUAL };

static State state = IDLE;
static Mode  mode  = AUTO;

// Status limit switch beserta variabel untuk debounce
static bool switchAktif       = false;
static bool switchBacaan      = false;
static unsigned long waktuBerubah = 0;

// Kondisi mode manual
static bool motorManualOn = false;
static int  pwmManual     = 150;

// Dipakai buat histeresis di mode automatic
static bool motorNyalaAuto = false;

static unsigned long waktuCetak = 0;
static unsigned long waktuLed   = 0;
static bool ledNyala = false;

static const char* namaState() {
  switch (state) {
    case IDLE:       return "IDLE";
    case AUTO_RUN:   return "AUTO_RUN";
    case MANUAL_RUN: return "MANUAL_RUN";
    default:         return "FAULT";
  }
}

// Limit switch itu kontak mekanis, pas ditekan sinyalnya mantul-mantul
// beberapa milidetik. Tanpa debounce, satu tekanan kebaca sebagai
// belasan perubahan state.
static void bacaSwitch() {
  bool bacaan = (digitalRead(PIN_LIMIT_SWITCH) == LOW);   // aktif LOW

  if (bacaan != switchBacaan) {
    switchBacaan  = bacaan;
    waktuBerubah  = millis();
    return;
  }
  if (switchAktif != bacaan && millis() - waktuBerubah >= DEBOUNCE_SWITCH) {
    switchAktif = bacaan;
  }
}

// Konversi suhu ke nilai PWM.
// Dipetakan ke PWM_MIN..PWM_MAX, bukan 0..255, karena di bawah PWM_MIN
// motor DC nggak muter, cuma narik arus.
static int pwmDariSuhu(float suhu) {
  // Histeresis: kalau motor sudah nyala, batas matinya dibuat lebih rendah
  // supaya suhu yang naik-turun tipis di sekitar 40 derajat nggak bikin
  // motor nyala-mati terus.
  float batas = motorNyalaAuto ? (SUHU_MULAI - SUHU_HISTERESIS) : SUHU_MULAI;

  if (suhu < batas) {
    motorNyalaAuto = false;
    return 0;
  }
  motorNyalaAuto = true;

  if (suhu >= SUHU_PENUH) return PWM_MAX;

  float rasio = (suhu - batas) / (SUHU_PENUH - batas);
  return PWM_MIN + (int)(rasio * (PWM_MAX - PWM_MIN));
}

// Satu LED dipakai buat empat state, jadi dibedain lewat pola kedipnya.
static void updateLed() {
  unsigned long interval = 0;

  switch (state) {
    case AUTO_RUN:                       // nyala terus
      digitalWrite(PIN_LED_STATUS, HIGH);
      ledNyala = true;
      return;
    case MANUAL_RUN: interval = 500; break;   // kedip lambat
    case FAULT:      interval = 100; break;   // kedip cepat
    case IDLE:                                // mati
      digitalWrite(PIN_LED_STATUS, LOW);
      ledNyala = false;
      return;
  }

  if (millis() - waktuLed >= interval) {
    waktuLed = millis();
    ledNyala = !ledNyala;
    digitalWrite(PIN_LED_STATUS, ledNyala ? HIGH : LOW);
  }
}

static void masukFault() {
  state = FAULT;
  motorStop();
  Serial.print("[FAULT] pembacaan sensor bermasalah (");
  Serial.print(sensorKeterangan());
  Serial.println("), motor dimatikan");
}

void kontrolInit() {
  pinMode(PIN_LIMIT_SWITCH, INPUT_PULLUP);
  pinMode(PIN_LED_STATUS, OUTPUT);
  state = IDLE;
  mode  = AUTO;
}

void kontrolUpdate() {
  bacaSwitch();
  sensorUpdate();

  // Pengecekan sensor berlaku di state mana pun, jadi ditaruh di luar switch.
  if (sensorBermasalah() && state != FAULT) {
    masukFault();
  }

  switch (state) {

    case IDLE:
      motorStop();
      if (mode == AUTO && switchAktif) {
        state = AUTO_RUN;
        motorNyalaAuto = false;
        Serial.println("[STATE] IDLE -> AUTO_RUN");
      } else if (mode == MANUAL && motorManualOn) {
        state = MANUAL_RUN;
        Serial.println("[STATE] IDLE -> MANUAL_RUN");
      }
      break;

    case AUTO_RUN:
      // Limit switch di sini fungsinya izin operasi. Begitu lepas, berhenti.
      if (mode != AUTO || !switchAktif) {
        motorStop();
        state = IDLE;
        Serial.println("[STATE] AUTO_RUN -> IDLE (limit switch lepas / ganti mode)");
        break;
      }
      motorSetPwm(pwmDariSuhu(sensorSuhu()));
      break;

    case MANUAL_RUN:
      if (mode != MANUAL || !motorManualOn) {
        motorStop();
        state = IDLE;
        Serial.println("[STATE] MANUAL_RUN -> IDLE");
        break;
      }
      motorSetPwm(pwmManual);
      break;

    case FAULT:
      // Fault sengaja nggak hilang sendiri. Harus direset operator pakai 'R',
      // biar masalah yang kadang muncul kadang hilang nggak lewat begitu aja.
      motorStop();
      break;
  }

  updateLed();

  if (millis() - waktuCetak >= INTERVAL_CETAK_STATUS) {
    waktuCetak = millis();
    kontrolCetakStatus();
  }
}

void kontrolPerintah(char c) {
  c = toupper(c);

  // Kalau lagi FAULT, semua perintah selain reset dan status ditolak.
  if (state == FAULT && c != 'R' && c != 'S' && c != 'H') {
    Serial.println("[TOLAK] sistem lagi FAULT, kirim 'R' dulu untuk reset");
    return;
  }

  switch (c) {

    case 'A':
      mode = AUTO;
      motorManualOn = false;
      state = IDLE;
      motorStop();
      Serial.println("[MODE] AUTOMATIC");
      break;

    case 'M':
      mode = MANUAL;
      motorManualOn = false;
      state = IDLE;
      motorStop();
      Serial.println("[MODE] MANUAL");
      break;

    case '1':
      if (mode != MANUAL) {
        Serial.println("[TOLAK] perintah motor manual cuma berlaku di mode MANUAL");
        break;
      }
      motorManualOn = true;
      Serial.println("[CMD] motor manual ON");
      break;

    case '0':
      if (mode != MANUAL) {
        Serial.println("[TOLAK] perintah motor manual cuma berlaku di mode MANUAL");
        break;
      }
      motorManualOn = false;
      Serial.println("[CMD] motor manual OFF");
      break;

    case 'R':
      if (state != FAULT) {
        Serial.println("[INFO] nggak ada fault yang aktif");
        break;
      }
      // Reset cuma boleh kalau sensornya sudah normal lagi, biar operator
      // nggak sekadar menghapus gejalanya.
      if (!sensorValid()) {
        Serial.print("[TOLAK] sensor masih bermasalah (");
        Serial.print(sensorKeterangan());
        Serial.println("), reset dibatalkan");
        break;
      }
      sensorResetGagal();
      state = IDLE;
      Serial.println("[INFO] fault direset, kembali ke IDLE");
      break;

    case 'S':
      kontrolCetakStatus();
      break;

    case 'H':
      kontrolCetakBantuan();
      break;

    default:
      Serial.print("[TOLAK] perintah nggak dikenal: ");
      Serial.println(c);
      break;
  }
}

void kontrolCetakStatus() {
  Serial.print("[STATUS] state=");   Serial.print(namaState());
  Serial.print(" mode=");            Serial.print(mode == AUTO ? "AUTO" : "MANUAL");
  Serial.print(" switch=");          Serial.print(switchAktif ? "AKTIF" : "LEPAS");
  Serial.print(" sensor=");          Serial.print(sensorKeterangan());
  Serial.print(" suhu=");            Serial.print(sensorSuhu(), 1);
  Serial.print("C pwm=");            Serial.print(motorPwmSekarang());
  Serial.print(" gagal=");           Serial.println(sensorJumlahGagal());
}

void kontrolCetakBantuan() {
  Serial.println("-----------------------------------------");
  Serial.println(" A  : mode AUTOMATIC");
  Serial.println(" M  : mode MANUAL");
  Serial.println(" 1  : motor ON  (mode MANUAL)");
  Serial.println(" 0  : motor OFF (mode MANUAL)");
  Serial.println(" R  : reset fault");
  Serial.println(" S  : lihat status");
  Serial.println(" H  : bantuan");
  Serial.println("-----------------------------------------");
}