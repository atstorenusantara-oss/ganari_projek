# TFT Random Dashboard (ESP32 + ILI9225)

Project dashboard kualitas udara sederhana berbasis **ESP32** dengan tampilan:
- **TFT ILI9225** (nilai CO2, O2, PM2.5, suhu, VOC)
- **Web dashboard** via WiFi (`WebServer`)

Nilai sensor saat ini masih **simulasi random** (placeholder), siap diganti ke sensor asli.

## Fitur
- Status koneksi WiFi tampil di TFT (CONNECTING, SSID, progress).
- Setelah terhubung, IP address ditampilkan 5 detik.
- Update data non-blocking tiap ~3 detik (`millis()`), bukan `delay` di loop.
- Web dashboard sensor di `/` auto-refresh tiap ~3.2 detik.
- Halaman pengaturan WiFi terpisah di `/wifi` (2 textbox + tombol update).
- SSID/password yang diupdate dari web disimpan ke **EEPROM** (persisten setelah restart/reset).
- Setelah simpan credential, board otomatis reconnect ke WiFi baru.
- Jika gagal connect WiFi selama 1 menit saat startup, WiFi dimatikan otomatis (`WIFI_OFF`).
- Jika reconnect WiFi gagal selama 1 menit, WiFi juga dimatikan otomatis (`WIFI_OFF`).
- Status WiFi ditampilkan di TFT bagian bawah kiri: `WiFi: ON` (hijau) / `WiFi: OFF` (merah).
- Mode cek via power toggle: nyala/mati 3 kali cepat (<3 detik per siklus) akan masuk `CHECK MODE`.
- Di `CHECK MODE`, TFT menampilkan SSID dan password WiFi yang tersimpan.
- Kode sudah dipisah ke beberapa file agar modular.

## Struktur File
- `TFT_random_dashboard.ino` -> flow utama (`setup`, `loop`, route web)
- `dashboard_ui.h/.cpp` -> fungsi render TFT
- `sensor_data.h/.cpp` -> model + update data sensor random
- `web_dashboard.h/.cpp` -> fungsi kirim HTML dashboard
- `platformio.ini` -> konfigurasi PlatformIO

## Hardware
- Board: ESP32 (contoh: `esp32dev`)
- Display: TFT 2.2 ILI9225 (SPI)

Pin default di kode:
- `TFT_CS = 5`
- `TFT_RST = 4`
- `TFT_RS = 2`
- `TFT_LED = 255`

## Software Requirement
- VS Code
- Extension: PlatformIO IDE
- Driver USB sesuai board (CP210x / CH340)

## Setup
1. Buka folder project ini di VS Code.
2. (Opsional) Edit WiFi default di `TFT_random_dashboard.ino`:
   - `defaultSsid`
   - `defaultPassword`
3. Pastikan board di `platformio.ini` sesuai board kamu.

Catatan:
- Jika sudah pernah set WiFi dari halaman `/wifi`, maka nilai dari EEPROM dipakai saat boot (mengabaikan default hardcode).
- Jika WiFi masuk status `OFF` karena timeout 1 menit, restart/power on ulang akan menyalakan WiFi lagi dan mencoba konek ulang.

## Mode Khusus
- `CHECK MODE`:
  - Trigger: power on/off cepat 3 kali sebelum board sempat hidup stabil >3 detik.
  - Fungsi: menampilkan credential WiFi (SSID/password) di TFT untuk pengecekan cepat.
  - Keluar mode: nyalakan board normal (tanpa 3x power toggle cepat).

## Verify / Build
Jalankan di terminal:

```powershell
pio run
```

Jika berhasil, output akhir akan menunjukkan `SUCCESS`.

## Upload ke ESP32

```powershell
pio run -t upload
```

## Upload OTA (WiFi)
1. Upload pertama tetap lewat USB.
2. Pastikan Serial Monitor menampilkan IP device dan host OTA (`ganari-dashboard.local`).
3. Aktifkan konfigurasi OTA di `platformio.ini`:
   - `upload_protocol = espota`
   - `upload_port = <IP-ESP32>`
4. Jalankan upload seperti biasa:

```powershell
pio run -t upload
```

## Upload Firmware Lewat Browser (HTTP OTA)
1. Upload awal tetap lewat USB sekali.
2. Buka browser ke `http://<IP-ESP32>/update`.
3. Login:
   - user: `admin`
   - password: `admin123`
4. Pilih file firmware `.bin` hasil build:
   - `.pio/build/esp32dev/firmware.bin`
5. Klik Upload, tunggu selesai, board akan restart otomatis.

Catatan:
- Ganti user/password OTA HTTP di `TFT_random_dashboard.ino` sebelum dipakai di jaringan publik.

## Endpoint Web
- `GET /` -> dashboard sensor utama
- `GET /wifi` -> halaman update SSID/password WiFi
- `POST /wifi-save` -> simpan SSID/password ke EEPROM + reconnect WiFi
- `GET /update` -> halaman upload firmware (HTTP OTA, basic auth)

Jika perlu monitor serial:

```powershell
pio device monitor -b 115200
```

## Catatan Penting
- Build pertama butuh internet untuk download package/library PlatformIO.
- Jika port tidak terdeteksi, set manual di `platformio.ini`:
  - `upload_port = COMx`
  - `monitor_port = COMx`
- Jika upload macet di "Connecting...", tekan tombol `BOOT` di board saat proses upload dimulai.

## Next Step (Opsional)
- Ganti `updateSensorDataRandom()` dengan pembacaan sensor real.
- Ubah web jadi endpoint JSON + fetch agar tidak full page reload.
- Tambah validasi panjang SSID/password dan proteksi auth untuk endpoint `/wifi-save`.
