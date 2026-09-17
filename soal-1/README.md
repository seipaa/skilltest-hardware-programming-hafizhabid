# Soal 1 — Hardware Control & Programming Logic

Kontrol motor DC berbasis sensor suhu dan limit switch, dengan mode Automatic
dan Manual, LED indikator status, dan penanganan kondisi abnormal.

**Board:** ESP32 DevKit V1
**Bahasa:** C++ (Arduino framework)
**Simulator:** Wokwi

## Cara Menjalankan

### Lewat Wokwi (tanpa hardware)

1. Buka simulasi: ( [link Wokwi](https://wokwi.com/projects/475368178705855489) )
2. Klik **Start Simulation**
3. Buka **Serial Monitor**, pastikan baud rate **115200**
4. Ketik `H` untuk lihat daftar perintah
5. Putar potensiometer untuk mengubah suhu, tekan pushbutton untuk limit switch

### Lewat Arduino IDE (hardware fisik)

1. Install ESP32 board package lewat Board Manager
2. Buka `soal-1.ino`, pastikan semua file `.h` dan `.cpp` ada di folder yang sama
3. Pilih board **ESP32 Dev Module**, upload
4. Buka Serial Monitor di 115200, line ending **No line ending**

Kode ini ditulis untuk **ESP32 Arduino core 2.x** yang pakai
`ledcSetup()` / `ledcAttachPin()`. Kalau pakai core 3.x, ganti dua baris itu
di `motor.cpp` jadi `ledcAttach(PIN_MOTOR_PWM, PWM_FREQ, PWM_RESOLUSI)`
dan `ledcWrite(PIN_MOTOR_PWM, pwm)`.

## Wiring / Pin Mapping

| Pin ESP32 | Komponen | Keterangan |
|---|---|---|
| GPIO34 | Sensor suhu (LM35) | Pin input-only, ADC1 |
| GPIO14 | Limit switch | `INPUT_PULLUP`, aktif LOW |
| GPIO25 | Motor PWM | Ke pin ENA driver motor (L298N) |
| GPIO27 | LED status | Seri dengan resistor 220 ohm |
| 3V3 | VCC sensor | |
| GND | Ground bersama | Sensor, switch, LED, driver |

```
      ESP32 DevKit V1
   ┌──────────────────────┐
   │ 3V3 ──── VCC sensor  │
   │ D34 ──── OUT sensor  │  (LM35, 10mV/C)
   │ D14 ──── limit switch ── GND
   │ D25 ──── ENA driver motor
   │ D27 ──── LED ── 220R ── GND
   │ GND ──── ground bersama
   └──────────────────────┘
```

Untuk hardware fisik, ground driver motor harus nyambung ke ground ESP32,
tapi jalur daya motor sebaiknya dipisah dari daya logika supaya arus start
motor nggak bikin tegangan ESP32 drop.

## Perintah Operator

Perintah dikirim lewat Serial Monitor:

| Perintah | Fungsi |
|---|---|
| `A` | Pindah ke mode AUTOMATIC |
| `M` | Pindah ke mode MANUAL |
| `1` | Motor ON (cuma berlaku di mode MANUAL) |
| `0` | Motor OFF (cuma berlaku di mode MANUAL) |
| `R` | Reset fault |
| `S` | Lihat status sekarang |
| `H` | Bantuan |

## Penjelasan Logic

### State Machine

Sistem punya empat state:

```
   ┌──────┐   mode AUTO + limit switch aktif     ┌──────────┐
   │ IDLE │ ──────────────────────────────────>  │ AUTO_RUN │
   │      │ <──────────────────────────────────  │          │
   │      │   switch lepas / ganti mode          └──────────┘
   │      │
   │      │   mode MANUAL + perintah '1'         ┌────────────┐
   │      │ ──────────────────────────────────>  │ MANUAL_RUN │
   │      │ <──────────────────────────────────  │            │
   └──┬───┘   perintah '0' / ganti mode          └────────────┘
      │
      │  sensor gagal 5x berturut-turut (dari state mana pun)
      v
   ┌───────┐
   │ FAULT │ ──── perintah 'R' + sensor sudah normal ───> IDLE
   └───────┘
```

Alasan pakai state machine dan bukan flag boolean: ada dua mode operasi plus
kemungkinan fault, jadi kalau pakai flag ada kombinasi kondisi yang gampang
kelewat dan nggak pernah diuji. Dengan state machine, transisi yang nggak sah
nggak mungkin terjadi karena setiap perpindahan ditulis eksplisit.

### Mode Automatic

1. Limit switch harus dalam kondisi aktif. Di sini switch diperlakukan sebagai
   izin operasi, bukan tombol start, jadi kalau switch lepas saat motor jalan
   sistem langsung balik ke IDLE dan motor berhenti.
2. Suhu dibaca tiap 200 ms.
3. Suhu dipetakan ke PWM:
   - di bawah 40 C, PWM 0 (motor mati)
   - 40 sampai 80 C, naik linier dari PWM 80 sampai 255
   - di atas 80 C, PWM 255
4. Ada histeresis 2 C. Setelah motor nyala, batas matinya turun jadi 38 C,
   supaya suhu yang naik-turun tipis di sekitar 40 C nggak bikin motor
   nyala-mati berulang kali.
5. PWM dipetakan mulai dari 80, bukan 0, karena motor DC nggak muter di duty
   cycle terlalu rendah, cuma dengung dan narik arus.

### Mode Manual

Motor dikontrol lewat perintah serial `1` dan `0`. Nilai PWM manual dipatok
di 150. Perintah motor manual ditolak kalau sistem lagi di mode Automatic.

### LED Indikator

| Pola | Arti |
|---|---|
| Mati | IDLE |
| Nyala terus | AUTO_RUN |
| Kedip lambat (500 ms) | MANUAL_RUN |
| Kedip cepat (100 ms) | FAULT |

## Penanganan Kondisi Abnormal

| Kondisi | Cara dideteksi | Respons sistem |
|---|---|---|
| Kabel sensor lepas atau korslet | Nilai ADC mentok di 0 atau 4095 | Hitung gagal, 5x berturut-turut masuk FAULT |
| Hasil konversi nggak masuk akal | Suhu di luar -10 sampai 150 C | Sama seperti di atas |
| Limit switch lepas saat motor jalan | Pengecekan tiap loop di state AUTO_RUN | Motor berhenti, balik ke IDLE |
| Kontak switch mantul | Debounce 30 ms | Nggak terjadi perpindahan state ganda |
| Perintah nggak sesuai mode | Dicek di `kontrolPerintah()` | Ditolak, ada pesan alasannya, state nggak berubah |
| Perintah saat FAULT | Dicek di awal `kontrolPerintah()` | Semua perintah selain `R`, `S`, `H` ditolak |
| Reset fault padahal sensor masih rusak | Cek `sensorValid()` sebelum reset | Reset dibatalkan |

Dua keputusan yang perlu dijelaskan:

**Kenapa perlu gagal 5x dulu.** Satu pembacaan ADC yang meleset itu wajar,
namanya noise. Kalau satu pembacaan aneh langsung bikin FAULT, motor bakal
berhenti tanpa sebab yang jelas. Lima kali berturut-turut (sekitar 1 detik)
itu baru kegagalan beneran.

**Kenapa fault harus direset manual.** Kalau fault hilang sendiri begitu sensor
normal lagi, masalah yang kadang muncul kadang hilang nggak akan pernah
ketahuan. Dengan reset manual, operator pasti tahu pernah terjadi fault.
Reset juga ditolak kalau sensornya masih bermasalah.

## Asumsi

1. Sensor suhu diasumsikan LM35 dengan output 10 mV per derajat Celsius.
   Kalau ganti sensor lain, yang perlu diubah cuma fungsi `adcKeCelsius()`
   di `sensor.cpp`.
2. Limit switch dipasang aktif LOW dengan pull-up internal ESP32.
3. Motor cuma satu arah. Driver motor pakai L298N dengan pin IN1/IN2 diset
   tetap, dan ESP32 cuma mengatur kecepatan lewat pin ENA.
4. Threshold 40 C dan 80 C itu angka contoh karena soal nggak nyebut angka
   spesifik. Semua angka ditaruh di `config.h` biar gampang diubah.
5. Input operator lewat serial karena paling gampang diuji dan direproduksi
   tanpa nambah hardware.

## Keterbatasan Simulasi

| Komponen asli | Di Wokwi | Keterbatasan |
|---|---|---|
| Sensor LM35 | Potensiometer | Potensiometer dipilih karena bisa diputar sampai ujung, jadi kondisi ADC mentok (sensor lepas/korslet) bisa diuji. Modul sensor di simulator nggak bisa disuruh rusak. |
| Motor DC + driver | LED di pin PWM | Kecerahan LED mewakili duty cycle. Nggak mensimulasikan arus start, beban induktif, atau back-EMF. |
| Limit switch | Pushbutton | Perilaku bouncing tetap ada, jadi logika debounce tetap teruji. |

Kalau dipindah ke hardware fisik, yang perlu disesuaikan cuma fungsi
`adcKeCelsius()` di `sensor.cpp`. Seluruh state machine dan penanganan
fault nggak berubah.
