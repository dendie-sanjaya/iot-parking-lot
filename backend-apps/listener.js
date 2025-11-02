const mqtt = require('mqtt');
const sqlite3 = require('sqlite3').verbose();

// HANYA MEMBUAT KONEKSI KE DATABASE (Sesuai Permintaan)
// Menggunakan jalur relatif untuk database
const db = new sqlite3.Database('./parkir.db');

// --- 2. KONFIGURASI DAN KONEKSI MQTT ---
const MQTT_TOPIC = 'parkir/slot/update';
const MQTT_BROKER_URL = 'mqtt://192.168.100.35'; // Broker lokal Anda

// Koneksi ke MQTT Broker
const client = mqtt.connect(MQTT_BROKER_URL);

client.on('connect', () => {
    console.log(`[MQTT] Terhubung ke broker: ${MQTT_BROKER_URL}`);
    client.subscribe(MQTT_TOPIC, (err) => {
        if (!err) {
            console.log(`[MQTT] Berhasil subscribe ke topik: ${MQTT_TOPIC}`);
            console.log("=================================================");
            console.log("Simulasi: Kirim pesan JSON ke topik ini untuk menguji");
            console.log('Contoh: {"slot_kode": "A01", "status": 1}');
            console.log("=================================================");
        } else {
            console.error(`[MQTT] Gagal subscribe: ${err.message}`);
        }
    });
});

client.on('message', (topic, message) => {
    console.log(`\n[MQTT] Pesan diterima dari topik ${topic}: ${message.toString()}`);
    
    try {
        const payload = JSON.parse(message.toString());
        const { slot_kode, status } = payload; // Status harus 0 atau 1

        if (slot_kode && (status === 0 || status === 1)) {
            // Tentukan kolom waktu yang akan diisi
            const timestampColumn = status === 1 ? 'last_in' : 'last_out';
            const timestamp = new Date().toISOString();
            
            // Query untuk memperbarui status dan timestamp yang relevan
            const sql = `
                UPDATE slot_parkir 
                SET status = ?, 
                    ${timestampColumn} = ? 
                WHERE slot_kode = ?
            `;

            db.run(sql, [status, timestamp, slot_kode], function(err) {
                if (err) {
                    // Penting: Jika tabel belum dibuat (tidak ada initializeDatabase), ini akan error.
                    console.error(`[DB ERROR] Gagal update slot ${slot_kode}: Pastikan tabel 'slot_parkir' sudah ada.`, err.message);
                    return;
                }
                if (this.changes > 0) {
                    console.log(`[DB SUCCESS] Slot ${slot_kode} berhasil diubah statusnya menjadi ${status} pada waktu ${timestamp}.`);
                } else {
                    console.warn(`[DB WARNING] Slot ${slot_kode} tidak ditemukan dalam database.`);
                }
            });

        } else {
            console.warn('[PARSING] Payload tidak valid. Pastikan ada slot_kode dan status (0/1).');
        }

    } catch (e) {
        console.error('[PARSING] Gagal memparsing JSON payload:', e.message);
    }
});

client.on('error', (err) => {
    console.error(`[MQTT ERROR] Terjadi kesalahan: ${err.message}`);
});

client.on('offline', () => {
    console.warn('[MQTT] Client offline. Mencoba koneksi ulang...');
});
