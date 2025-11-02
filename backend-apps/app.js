const express = require('express');
const cors = require('cors');
const { initializeDatabase, db } = require('./database');

const app = express();
const PORT = 3000;

// Inisialisasi database dan mulai server setelahnya
initializeDatabase();

// Middleware
app.use(cors()); // Memungkinkan akses dari frontend (index.html)
app.use(express.json());

// ===================================
// ENDPOINT 1: Health Check
// ===================================
app.get('/health', (req, res) => {
    // API Sederhana untuk memeriksa apakah server berjalan
    res.json({ 
        status: 'OK', 
        message: 'Server parkir berjalan dengan sehat.', 
        timestamp: new Date().toISOString() 
    });
});

// ===================================
// ENDPOINT 2: Status Semua Slot
// ===================================
app.get('/api/slots', (req, res) => {
    // Query untuk mengambil semua data slot dari tabel
    const sql = "SELECT * FROM slot_parkir ORDER BY slot_kode ASC";

    db.all(sql, [], (err, rows) => {
        if (err) {
            console.error("Kesalahan saat mengambil data slot:", err.message);
            return res.status(500).json({ error: 'Gagal mengambil data dari database.' });
        }
        
        // Menghitung total slot
        const totalSlots = rows.length;
        
        // Menghitung total slot yang tersedia (status = 0)
        const totalAvailable = rows.filter(row => row.status === 0).length;

        // Mempersiapkan respons API
        const responseData = {
            total_slots: totalSlots,
            total_available: totalAvailable,
            total_occupied: totalSlots - totalAvailable,
            slots: rows.map(slot => ({
                slot_kode: slot.slot_kode,
                status: slot.status,
                status_text: slot.status === 1 ? 'Terisi' : 'Kosong',
                last_in: slot.last_in || 'N/A',
                last_out: slot.last_out || 'N/A'
            }))
        };

        res.json(responseData);
    });
});

// Mulai Server
app.listen(PORT, () => {
    console.log(`[API] Express Server berjalan di http://localhost:${PORT}`);
    console.log(`[API] Endpoint Health Check: http://localhost:${PORT}/health`);
    console.log(`[API] Endpoint Data Slot: http://localhost:${PORT}/api/slots`);
});
