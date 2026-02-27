# Dokumentasi Lengkap

## 1. Ringkasan Proyek
`TFT_random_dashboard` adalah proyek ESP32 untuk menampilkan data kualitas udara simulasi di:
- TFT ILI9225 (lokal di device)
- Web dashboard (melalui WiFi)

Data sensor saat ini masih simulasi random dan sudah disiapkan agar mudah diganti ke sensor real.

## 2. Fitur Utama
- Tampilan parameter di TFT: CO2, O2, PM2.5, Temp, VOC.
- Header dashboard di TFT.
- Dashboard web utama di `/` (auto-refresh).
- Halaman konfigurasi WiFi terpisah di `/wifi`.
- Simpan SSID/password dari web ke EEPROM (persisten setelah restart).
- Reconnect otomatis setelah update credential.
- HTTP OTA firmware via `/update` (dengan basic auth).
- Check mode: power on/off cepat 3x (<3 detik) menampilkan SSID/password di TFT.
- Proteksi koneksi: jika gagal connect/reconnect selama 1 menit, WiFi dimatikan (`WIFI_OFF`).
- Status WiFi di TFT (bawah kiri): `WiFi: ON` (hijau) atau `WiFi: OFF` (merah).

## 3. Struktur File
- `TFT_random_dashboard.ino`: alur utama aplikasi, setup, loop, routing web, state WiFi, EEPROM.
- `dashboard_ui.h/.cpp`: fungsi render UI di TFT.
- `sensor_data.h/.cpp`: model data sensor + generator random.
- `web_dashboard.h/.cpp`: HTML dashboard web dan halaman setting WiFi.
- `platformio.ini`: konfigurasi build PlatformIO.
- `README.md`: ringkasan proyek.

## 4. Arsitektur Alur Sistem
1. Boot ESP32.
2. Inisialisasi TFT + EEPROM.
3. Baca SSID/password dari EEPROM, fallback ke default hardcoded jika kosong.
4. Cek rapid power toggle counter:
   - Jika mencapai 3, masuk `CHECK MODE` dan tampilkan credential di TFT.
   - Jika tidak, lanjut koneksi WiFi.
5. Coba koneksi WiFi:
   - Jika berhasil, aktifkan OTA + web server.
   - Jika gagal >1 menit, matikan WiFi (`WIFI_OFF`) dan lanjut mode lokal TFT.
6. Loop utama:
   - Update data sensor tiap ~3 detik.
   - Render data ke TFT.
   - Jika WiFi aktif, layani web/OTA.
   - Jika reconnect gagal >1 menit, matikan WiFi.

## 5. Endpoint Web
- `GET /`
  - Dashboard sensor utama.
- `GET /wifi`
  - Halaman form update SSID/password.
- `POST /wifi-save`
  - Simpan SSID/password ke EEPROM dan trigger reconnect.
- `GET /ping`
  - Health check sederhana (`pong`).
- `GET /update`
  - Halaman upload firmware OTA via browser (basic auth).
- `POST /update`
  - Upload firmware OTA + restart otomatis jika sukses.

## 6. Penyimpanan EEPROM
Penyimpanan menggunakan EEPROM emulation ESP32.

Map address yang dipakai:
- SSID:
  - start: `0`
  - panjang slot: `32`
- Password:
  - start: `32`
  - panjang slot: `64`
- Rapid power-toggle counter:
  - address: `120`

Ukuran EEPROM yang diinisialisasi:
- `EEPROM_SIZE = 128`

Catatan:
- SSID/password disimpan null-terminated.
- `EEPROM.commit()` dipanggil saat menulis agar perubahan permanen.

## 7. Check Mode (3x Power Cycle Cepat)
Tujuan: recovery/cek credential ketika device sulit diakses lewat jaringan.

Mekanisme:
- Setiap boot, counter di EEPROM dinaikkan.
- Jika uptime sudah lewat ~3 detik, counter di-reset ke 0.
- Jika sebelum reset terjadi boot cepat berulang hingga counter >= 3:
  - masuk `CHECK MODE`
  - tampilkan SSID/password di TFT
  - loop utama dihentikan (mode display-only)

Cara keluar:
- Nyalakan board normal (tanpa 3x power cycle cepat).

## 8. Manajemen WiFi Timeout
Logika timeout:
- Startup connect timeout:
  - Jika belum connect >60 detik sejak `WiFi.begin`, maka WiFi dimatikan.
- Reconnect timeout:
  - Saat koneksi putus, device mencoba reconnect periodik.
  - Jika tetap gagal >60 detik, WiFi dimatikan.

Saat WiFi dimatikan:
- `WiFi.mode(WIFI_OFF)` dipanggil.
- Web server/OTA tidak berjalan.
- TFT tetap update data sensor lokal.
- Status TFT menunjukkan `WiFi: OFF` merah.

Perilaku setelah restart:
- WiFi kembali aktif saat boot.
- Device mencoba connect lagi normal.

## 9. UI TFT
Screen yang tersedia:
- Header dashboard.
- WiFi connecting screen (SSID + progress animasi titik).
- WiFi connected screen (IP address).
- Check mode screen (SSID/password).
- Sensor dashboard screen.

Informasi status koneksi:
- Ditampilkan di area bawah kiri pada screen sensor:
  - `WiFi: ON` hijau.
  - `WiFi: OFF` merah.

## 10. UI Web
Halaman `/`:
- Menampilkan kartu parameter sensor.
- Auto-refresh tiap ~3.2 detik.
- Tautan ke halaman `/wifi`.

Halaman `/wifi`:
- Form input SSID.
- Form input password.
- Tombol `Update WiFi`.
- Setelah submit:
  - credential disimpan ke EEPROM
  - board reconnect ke AP baru

## 11. Build dan Upload
Prerequisite:
- VS Code
- PlatformIO IDE extension
- Driver USB board (CP210x/CH340)

Build:
```powershell
pio run
```

Upload via USB:
```powershell
pio run -t upload
```

Serial monitor:
```powershell
pio device monitor -b 115200
```

## 12. OTA Firmware (Browser)
Langkah:
1. Upload awal firmware via USB.
2. Buka `http://<ip-esp>/update`.
3. Login basic auth.
4. Upload file:
   - `.pio/build/esp32dev/firmware.bin`
5. Jika sukses board restart otomatis.

Default kredensial OTA HTTP:
- user: `admin`
- pass: `admin123`

## 13. Konfigurasi Hardware
Board default:
- `esp32dev`

Pin TFT default:
- `TFT_CS = 5`
- `TFT_RST = 4`
- `TFT_RS = 2`
- `TFT_LED = 255`

## 14. Dependency
Library utama:
- `TFT_22_ILI9225`
- `WiFi`
- `WebServer`
- `ArduinoOTA`
- `Update`
- `EEPROM`
- `SPI`

Semua dikelola oleh PlatformIO melalui `platformio.ini` + package framework ESP32.

## 15. Cara Ganti Data Random ke Sensor Real
Titik integrasi:
- `sensor_data.h` untuk struktur data.
- `sensor_data.cpp` pada `updateSensorDataRandom(...)`.

Strategi migrasi:
1. Tambah library sensor yang dibutuhkan.
2. Inisialisasi sensor di `setup()`.
3. Ganti isi fungsi update random dengan pembacaan sensor real.
4. Pertahankan interface struct agar UI TFT/Web tidak perlu banyak diubah.

## 16. Troubleshooting
- Gagal compile:
  - Jalankan `pio run -v` untuk detail.
- Port upload tidak terdeteksi:
  - set manual `upload_port` dan `monitor_port` di `platformio.ini`.
- Upload macet di `Connecting...`:
  - tekan tombol `BOOT` saat proses upload mulai.
- Tidak bisa akses dashboard:
  - cek serial monitor untuk IP.
  - uji `GET /ping`.
- WiFi tiba-tiba `OFF`:
  - ini normal jika gagal connect/reconnect >1 menit.
  - restart board untuk mencoba connect ulang.

## 17. Catatan Keamanan
- Ganti kredensial OTA HTTP default sebelum dipakai di jaringan produksi.
- Halaman `/wifi` saat ini belum diproteksi auth; disarankan tambah proteksi jika dipakai di jaringan publik.
- Hindari menaruh SSID/password sensitif hardcoded untuk deployment umum.

## 18. Roadmap Rekomendasi
- Tambahkan auth untuk endpoint `/wifi` dan `/wifi-save`.
- Tambahkan validasi panjang/karakter SSID-password.
- Tambahkan endpoint JSON (`/api/sensors`) agar dashboard bisa update tanpa full reload.
- Tambahkan logging ringkas status WiFi + reason disconnect.
- Tambahkan fitur reset credential via tombol fisik.
