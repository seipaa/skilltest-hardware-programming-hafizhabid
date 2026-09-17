# skilltest-hardware-programming-hafizhabid

Hasil pengerjaan take-home technical assessment untuk posisi **Hardware Programming**.

- **Nama:** Hafizh 'Abid Khalish
- **Email:** fizh.lish@gmail.com

## Isi Repository

| Folder | Soal | Deskripsi | Tech Stack |
|---|---|---|---|
| [`soal-1/`](soal-1/) | Soal 1 | Hardware control & programming logic | ESP32, Arduino IDE (C++), Wokwi |
| [`soal-2/`](soal-2/) | Soal 2 | Serial communication ke REST API | Node.js, Express, Axios, SerialPort |
| [`soal-3/`](soal-3/) | Soal 3 | Troubleshooting & root cause analysis | Dokumen analisis & mitigasi sistem |

---

## Quick Start — Menjalankan Soal 2 (Serial ke REST API)

Aplikasi **Soal 2** sudah dirancang siap uji (*Zero-Config Ready*) tanpa membutuhkan hardware fisik. Anda dapat menjalankannya **langsung dari root folder** tanpa perlu masuk ke dalam folder `soal-2`.

### 1. Install Dependencies (dari Root)
```bash
npm run install:soal-2
# atau perintah standar npm:
npm --prefix soal-2 install
```

### 2. Jalankan Pengujian (Buka 2 Terminal)

- **Terminal 1 — Jalankan Mock REST API:**
  ```bash
  npm run mock-api
  # atau: npm --prefix soal-2 run mock-api
  ```

- **Terminal 2 — Jalankan Bridge Controller:**
  ```bash
  npm start
  # atau: npm --prefix soal-2 start
  ```

*(Panduan lengkap, pengujian dengan hardware fisik, format data, dan penanganan kegagalan dapat dibaca pada [soal-2/README.md](soal-2/README.md))*

---

## Ringkasan Solusi Tiap Soal

### Soal 1 — Hardware Control & Logic
Sistem kontrol motor dan pembacaan sensor suhu berbasis state machine (`IDLE`, `AUTO_RUN`, `MANUAL_RUN`, `FAULT`) pada mikrokontroler ESP32 dengan simulasi Wokwi. Limit switch difungsikan sebagai izin operasi (*safety interlock*), histeresis suhu 2.0°C untuk mencegah osilasi motor, serta filtering 5x sample fault untuk meredam noise transien ADC.
👉 *Detail ada di [soal-1/README.md](soal-1/README.md)*.

### Soal 2 — Serial Communication ke REST API
Bridge controller Node.js modular yang membaca frame stream serial berformat NDJSON, memvalidasi boundary fisis sensor, dan mem-POST ke REST API dengan mekanisme *retry exponential backoff*, penanganan pemutusan serial otomatis (*auto-reconnect*), isolasi unhandled exceptions, serta mode simulasi (*mock serial* & *mock API*).
👉 *Detail ada di [soal-2/README.md](soal-2/README.md)*.

### Soal 3 — Troubleshooting & Root Cause Analysis
Analisis mendalam mengenai fenomena kegagalan sistem IoT di lapangan: investigasi akar masalah kebocoran memori (*memory leak* / akumulasi event listener) dan data korup/undefined akibat framing mismatch, metodologi troubleshooting bertahap, konfigurasi metrik/logging observabilitas, serta perbaikan kode preventif.
👉 *Detail ada di [SOAL 3 — Troubleshooting Root Cause Analysis.pdf](soal-3/SOAL 3 — Troubleshooting Root Cause Analysis.pdf)*.

