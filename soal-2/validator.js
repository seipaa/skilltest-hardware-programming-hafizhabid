// Batas nilai yang masuk akal untuk tiap field sensor.
// Data di luar batas ini dianggap rusak dan nggak diteruskan ke server.
const BATAS = {
    temp: { min: -40, max: 125 },
    humidity: { min: 0, max: 100 },
    pressure: { min: 300, max: 1100 },
};

/**
 * Parse satu baris JSON dari serial lalu cek isinya.
 * Fungsi ini nggak pernah throw, selalu balikin objek hasil,
 * supaya pemanggilnya nggak perlu dibungkus try-catch lagi.
 *
 * @param {string} baris
 * @returns {{valid: boolean, data?: object, error?: string}}
 */
function parseAndValidate(baris) {
    let data;

    try {
        data = JSON.parse(baris);
    } catch (err) {
        return { valid: false, error: 'JSON nggak bisa di-parse' };
    }

    if (typeof data !== 'object' || data === null || Array.isArray(data)) {
        return { valid: false, error: 'payload bukan objek JSON' };
    }

    const hasil = {};

    for (const field of Object.keys(BATAS)) {
        const nilai = data[field];

        // null dan undefined dicek terpisah karena Number(null) hasilnya 0,
        // jadi kalau nggak dicek di sini data kosong bakal lolos jadi angka 0.
        if (nilai === undefined || nilai === null) {
            return { valid: false, error: `field ${field} nggak ada atau null` };
        }

        const angka = Number(nilai);
        if (!isFinite(angka)) {
            return { valid: false, error: `field ${field} bukan angka: ${nilai}` };
        }

        const { min, max } = BATAS[field];
        if (angka < min || angka > max) {
            return { valid: false, error: `field ${field} di luar batas wajar: ${angka}` };
        }

        hasil[field] = angka;
    }

    return { valid: true, data: hasil };
}

module.exports = { parseAndValidate };