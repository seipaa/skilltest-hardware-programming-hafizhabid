const { EventEmitter } = require('events');
const config = require('./config');
const log = require('./logger');

// Semua baris yang sudah lengkap dilempar lewat bus ini,
// jadi index.js nggak perlu tahu datanya dari hardware asli atau dari mock.
const bus = new EventEmitter();

let timerReconnect = null;

/**
 * Buat koneksi serial.
 * Balikannya selalu { port, parser } supaya mode mock dan mode asli
 * bisa ditangani dengan kode yang sama.
 */
function buatKoneksi() {
    if (config.serial.port === 'mock') {
        const buatMockSerial = require('./mock/mock-serial');
        log.info('Mode mock: pakai device serial tiruan');
        return buatMockSerial();
    }

    // require ditaruh di sini, bukan di atas file, supaya mode mock tetap jalan
    // meskipun package serialport gagal ter-install di mesin tertentu.
    const { SerialPort } = require('serialport');
    const { ReadlineParser } = require('@serialport/parser-readline');

    const port = new SerialPort({
        path: config.serial.port,
        baudRate: config.serial.baudRate,
    });

    // Data serial itu stream byte, bukan pesan. Parser ini yang motong
    // stream jadi baris utuh berdasarkan newline.
    const parser = port.pipe(new ReadlineParser({ delimiter: '\n' }));

    return { port, parser };
}

function hubungkan() {
    let koneksi;

    try {
        koneksi = buatKoneksi();
    } catch (err) {
        log.error(`Gagal buka port serial: ${err.message}`);
        jadwalkanReconnect();
        return;
    }

    const { port, parser } = koneksi;

    port.on('open', () => {
        log.info(`Port serial terbuka: ${config.serial.port} @ ${config.serial.baudRate}`);
    });

    parser.on('data', (baris) => {
        const bersih = String(baris).trim();
        if (bersih.length > 0) bus.emit('data', bersih);
    });

    port.on('error', (err) => {
        log.error(`Error serial: ${err.message}`);
        bersihkan(port, parser);
        jadwalkanReconnect();
    });

    port.on('close', () => {
        log.warn('Port serial tertutup');
        bersihkan(port, parser);
        jadwalkanReconnect();
    });
}

/**
 * Lepas semua listener dari koneksi lama sebelum dibuang.
 * Kalau ini nggak dilakukan, tiap reconnect bakal nambah listener baru
 * di atas yang lama, dan lama-lama satu data kebaca berkali-kali.
 */
function bersihkan(port, parser) {
    port.removeAllListeners();
    parser.removeAllListeners();
    try {
        if (port.isOpen) port.close();
    } catch (err) {
        // port mungkin sudah tertutup duluan, nggak masalah
    }
}

function jadwalkanReconnect() {
    // Event 'error' dan 'close' sering muncul berbarengan untuk satu kejadian.
    // Guard ini bikin cuma ada satu reconnect yang terjadwal.
    if (timerReconnect) return;

    log.info(`Coba sambung ulang dalam ${config.serial.reconnectDelay} ms`);

    timerReconnect = setTimeout(() => {
        timerReconnect = null;
        hubungkan();
    }, config.serial.reconnectDelay);
}

module.exports = { bus, mulai: hubungkan };