# TFT Random Dashboard (ESP32 + ILI9225)

Project dashboard kualitas udara sederhana berbasis **ESP32** dengan tampilan:
- **TFT ILI9225** (nilai CO2, O2, PM2.5, suhu)
- **Web dashboard** via WiFi (`WebServer`)

Nilai sensor saat ini masih **simulasi random** (placeholder), siap diganti ke sensor asli.

## Fitur
- Status koneksi WiFi tampil di TFT (CONNECTING, SSID, progress).
- Setelah terhubung, IP address ditampilkan 5 detik.
- Update data non-blocking tiap ~3 detik (`millis()`), bukan `delay` di loop.
- Web dashboard auto-refresh tiap ~3.2 detik.
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
2. Edit WiFi credential di `TFT_random_dashboard.ino`:
   - `ssid`
   - `password`
3. Pastikan board di `platformio.ini` sesuai board kamu.

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
- Tambah auto-reconnect WiFi jika koneksi putus.
