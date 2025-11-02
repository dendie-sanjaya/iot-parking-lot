const { initializeDatabase, db } = require('./database');

// Inisialisasi database (membuat tabel jika belum ada)
initializeDatabase();

// Data awal untuk diisi
const initialData = [
    { slot_kode: '1', status: 0, last_in: null, last_out: new Date().toISOString() },
    { slot_kode: '2', status: 0, last_in: new Date().toISOString(), last_out: null },
    { slot_kode: '3', status: 0, last_in: null, last_out: new Date().toISOString() }
];

db.serialize(() => {
    // Hapus data lama untuk memastikan data awal bersih (opsional)
    db.run("DELETE FROM slot_parkir");
    
    // Siapkan pernyataan INSERT
    const stmt = db.prepare("INSERT INTO slot_parkir (slot_kode, status, last_in, last_out) VALUES (?, ?, ?, ?)");

    // Masukkan 3 data contoh
    initialData.forEach(data => {
        stmt.run(data.slot_kode, data.status, data.last_in, data.last_out, (err) => {
            if (err) {
                console.error(`Gagal memasukkan data ${data.slot_kode}:`, err.message);
            } else {
                console.log(`Data slot ${data.slot_kode} berhasil dimasukkan.`);
            }
        });
    });

    stmt.finalize(() => {
        console.log("Semua data awal telah dimasukkan. Database siap.");
        // db.close(); // Jangan tutup db agar proses async selesai
    });
});
