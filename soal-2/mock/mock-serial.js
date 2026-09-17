const { EventEmitter } = require('events');

/**
 * Device serial tiruan untuk pengujian tanpa hardware.
 *
 * Bentuk objeknya sengaja dibikin sama dengan serialport asli
 * (port punya event open/error/close, parser punya event data),
 * jadi serial.js nggak perlu tahu ini asli atau tiruan.
 *
 * Yang disimulasikan:
 *   - data normal
 *   - JSON kepotong
 *   - field null dan field hilang
 *   - nilai di luar batas wajar
 *   - bukan JSON sama sekali
 *   - koneksi putus setelah 30 detik, untuk menguji reconnect
 */
module.exports = function buatMockSerial() {
    const port = new EventEmitter();
    const parser = new EventEmitter();

    const contohRusak = [
        '{"temp":28.5,"humidity":65,"pressure":1012',
        '{"temp":null,"humidity":65,"pressure":1012}',
        '{"temp":28.5,"pressure":1012}',
        '{"temp":9999,"humidity":65,"pressure":1012}',
        'bukan json sama sekali',
    ];

    // Kasih jeda sedikit supaya listener di serial.js sempat terpasang
    setTimeout(() => port.emit('open'), 100);

    const timerKirim = setInterval(() => {
        let baris;

        // Sekitar 25% data sengaja dibikin rusak biar jalur error benar-benar kelewat
        if (Math.random() < 0.25) {
            baris = contohRusak[Math.floor(Math.random() * contohRusak.length)];
        } else {
            baris = JSON.stringify({
                temp: Number((25 + Math.random() * 10).toFixed(1)),
                humidity: Math.round(55 + Math.random() * 25),
                pressure: Math.round(1005 + Math.random() * 15),
            });
        }

        parser.emit('data', baris);
    }, 1000);

    // Sengaja putus setelah 30 detik untuk menguji logika reconnect
    const timerPutus = setTimeout(() => {
        clearInterval(timerKirim);
        port.emit('close');
    }, 30000);

    port.isOpen = true;
    port.close = () => {
        clearInterval(timerKirim);
        clearTimeout(timerPutus);
        port.isOpen = false;
    };

    return { port, parser };
};