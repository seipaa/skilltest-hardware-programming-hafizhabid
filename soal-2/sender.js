const axios = require('axios');
const config = require('./config');
const log = require('./logger');

const tidur = (ms) => new Promise((resolve) => setTimeout(resolve, ms));

/**
 * Kirim satu payload ke REST API.
 * Kalau gagal, diulang sampai API_MAX_RETRY kali dengan jeda yang makin lama.
 *
 * @returns {Promise<boolean>} true kalau berhasil terkirim
 */
async function kirimData(payload) {
    for (let percobaan = 1; percobaan <= config.api.maxRetry; percobaan++) {
        try {
            const res = await axios.post(config.api.url, payload, {
                timeout: config.api.timeout,   // wajib, biar request nggak menggantung selamanya
                headers: { 'Content-Type': 'application/json' },
            });

            log.info(`Terkirim ke API (status ${res.status}, percobaan ke-${percobaan})`);
            return true;

        } catch (err) {
            const status = err.response ? err.response.status : null;

            // Status 4xx artinya payload-nya yang ditolak server.
            // Diulang berapa kali pun hasilnya bakal sama, jadi langsung berhenti.
            if (status && status >= 400 && status < 500) {
                log.error(`API menolak data (status ${status}), nggak diulang`);
                return false;
            }

            log.warn(`Gagal kirim (percobaan ${percobaan}/${config.api.maxRetry}): ${err.message}`);

            // Jeda makin lama tiap percobaan, biar nggak makin membebani server
            // yang kemungkinan lagi bermasalah.
            if (percobaan < config.api.maxRetry) {
                await tidur(config.api.retryDelay * percobaan);
            }
        }
    }

    log.error('Retry habis, data ini dibuang');
    return false;
}

module.exports = { kirimData };