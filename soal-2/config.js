const path = require('path');
require('dotenv').config({ path: path.resolve(__dirname, '.env') });

// Semua konfigurasi dibaca dari environment variable, dengan nilai default
// supaya aplikasi tetap bisa jalan meskipun .env belum diisi lengkap.
module.exports = {
    serial: {
        port: process.env.SERIAL_PORT || 'mock',
        baudRate: parseInt(process.env.SERIAL_BAUD_RATE || '115200', 10),
        reconnectDelay: parseInt(process.env.SERIAL_RECONNECT_DELAY || '3000', 10),
    },
    api: {
        url: process.env.API_URL || 'http://localhost:10000/api/sensor-data',
        timeout: parseInt(process.env.API_TIMEOUT || '5000', 10),
        maxRetry: parseInt(process.env.API_MAX_RETRY || '3', 10),
        retryDelay: parseInt(process.env.API_RETRY_DELAY || '1000', 10),
    },
};