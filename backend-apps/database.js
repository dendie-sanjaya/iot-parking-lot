const sqlite3 = require('sqlite3').verbose();
// Menggunakan jalur relatif untuk database
const db = new sqlite3.Database('./parkir.db');

const CREATE_TABLE_SQL = `
CREATE TABLE IF NOT EXISTS slot_parkir (
    slot_kode TEXT PRIMARY KEY NOT NULL,
    status INTEGER NOT NULL, -- 1: Terisi, 0: Kosong
    last_in DATETIME,        -- Waktu terakhir terisi
    last_out DATETIME        -- Waktu terakhir kosong
);
`;

/**
 * Fungsi untuk menginisialisasi database (membuat tabel jika belum ada).
 * @returns {sqlite3.Database} Objek koneksi database.
 */
function initializeDatabase() {
    db.serialize(() => {
        db.run(CREATE_TABLE_SQL, (err) => {
            if (err) {
                console.error("Gagal membuat tabel:", err.message);
            } else {
                console.log("Database 'slot_parkir' siap.");
            }
        });
    });
    return db;
}

module.exports = {
    db,
    initializeDatabase
};
