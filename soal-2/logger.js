// Logger sederhana biar tiap baris log ada waktu dan levelnya.
// Formatnya dibikin satu baris supaya gampang di-grep kalau lagi cari masalah.

function waktu() {
    return new Date().toISOString().replace('T', ' ').substring(0, 19);
}

function tulis(level, pesan) {
    console.log(`[${waktu()}] [${level}] ${pesan}`);
}

module.exports = {
    info: (pesan) => tulis('INFO', pesan),
    warn: (pesan) => tulis('WARN', pesan),
    error: (pesan) => tulis('ERROR', pesan),
};