const express = require('express');

// REST API tiruan untuk menguji bridge tanpa server sungguhan.
// Sebagian request sengaja dibalas gagal supaya mekanisme retry kelihatan jalan.

const app = express();
app.use(express.json());

const PORT = process.env.MOCK_API_PORT || 10000;
const PELUANG_GAGAL = 0.2;

let totalDiterima = 0;

app.post('/api/sensor-data', (req, res) => {
    if (Math.random() < PELUANG_GAGAL) {
        console.log('[mock-api] 503 (sengaja digagalkan untuk uji retry)');
        return res.status(503).json({ error: 'server lagi sibuk' });
    }

    totalDiterima++;
    console.log(`[mock-api] 200 #${totalDiterima}`, JSON.stringify(req.body));
    res.json({ ok: true, id: totalDiterima });
});

app.listen(PORT, () => {
    console.log(`[mock-api] siap di http://localhost:${PORT}/api/sensor-data`);
    console.log(`[mock-api] ${PELUANG_GAGAL * 100}% request akan dibalas 503`);
});