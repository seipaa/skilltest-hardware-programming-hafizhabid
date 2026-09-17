#ifndef CONFIG_H
#define CONFIG_H

// Semua pin dan angka threshold dikumpulin di sini biar kalau mau tuning
// nggak perlu ubah-ubah file lain.

// ---- Pin ESP32 ----
#define PIN_SENSOR_SUHU    34   // ADC1 channel 6, pin input-only
#define PIN_LIMIT_SWITCH   14   // pakai pull-up internal, aktif LOW
#define PIN_MOTOR_PWM      25   // ke pin ENA driver motor (L298N)
#define PIN_LED_STATUS     27

// ---- PWM ESP32 (LEDC) ----
#define PWM_CHANNEL        0
#define PWM_FREQ           5000
#define PWM_RESOLUSI       8    // 8 bit -> nilai duty 0..255

// ---- Batas kerja motor ----
#define PWM_MIN            80   // di bawah ini motor cuma dengung, nggak muter
#define PWM_MAX            255

// ---- Threshold suhu (Celsius) ----
#define SUHU_MULAI         40.0   // motor mulai jalan
#define SUHU_PENUH         80.0   // motor PWM penuh
#define SUHU_HISTERESIS    2.0    // biar nggak start-stop terus di sekitar batas

// ---- Batas kewajaran pembacaan sensor ----
#define SUHU_MIN_WAJAR     -10.0
#define SUHU_MAX_WAJAR     150.0
#define MAX_GAGAL_SENSOR   5      // gagal 5x berturut-turut baru dianggap FAULT

// ---- Timing (ms) ----
#define INTERVAL_BACA_SENSOR   200
#define INTERVAL_CETAK_STATUS  2000
#define DEBOUNCE_SWITCH        30

#define BAUD_RATE          115200

#endif