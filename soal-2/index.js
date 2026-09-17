const serial = require('./serial');
const { parseAndValidate } = require('./validator');
const { kirimData } = require('./sender');
const config = require('./config');
const log = require('./logger');

// Hitungan sederhana buat lihat kondisi sistem tanpa harus baca semua log
const statistik = {
    diterima: 0,
    valid: 0,
    tidakValid: 0,
    terkirim: 0,
    gagalKirim: 0,
};

// Alur utama: tiap baris yang masuk dari serial diparse, divalidasi,
// lalu kalau lolos baru dikirim ke API.
serial.bus.on('data', async (baris) => {
    statistik.diterima++;

    const hasil = parseAndValidate(baris);

    if (!hasil.valid) {
        // Data rusak cuma dicatat lalu dibuang. Aplikasi tetap jalan,
        // sesuai ketentuan soal nomor 8.
        statistik.tidakValid++;
        log.warn(`Data nggak valid (${hasil.error}) -> ${baris.substring(0, 60)}`);
        return;
    }

    statistik.valid++;

    // receivedAt ditambahkan di sisi bridge supaya server tahu kapan data
    // ini diterima, terpisah dari kapan datanya sampai di server.
    const payload = {
        ...hasil.data,
        receivedAt: new Date().toISOString(),
    };

    const sukses = await kirimData(payload);
    if (sukses) statistik.terkirim++;
    else statistik.gagalKirim++;
});

// Ringkasan tiap 30 detik
setInterval(() => {
    log.info(`Statistik: ${JSON.stringify(statistik)}`);
}, 30000);

// Jaring pengaman. Tanpa ini, satu error yang nggak ketangkap bisa
// mematikan proses, padahal bridge harus jalan terus.
process.on('unhandledRejection', (alasan) => {
    log.error(`Promise gagal tanpa ditangani: ${alasan}`);
});

process.on('uncaughtException', (err) => {
    log.error(`Error nggak tertangani: ${err.message}`);
});

process.on('SIGINT', () => {
    log.info(`Berhenti. Statistik akhir: ${JSON.stringify(statistik)}`);
    process.exit(0);
});

log.info('Bridge controller mulai');
log.info(`Serial: ${config.serial.port} | API: ${config.api.url}`);

serial.mulai();