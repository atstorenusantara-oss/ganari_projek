# Dokumentasi Pin I/O ESP32 dan TFT ILI9225 (Untuk PCB)

Dokumen ini merangkum koneksi pin berdasarkan implementasi di kode proyek saat ini.

## 1. Pin ESP32 yang Dipakai di Kode

Sumber: `TFT_random_dashboard.ino`

- `TFT_CS = GPIO5`
- `TFT_RST = GPIO4`
- `TFT_RS/DC = GPIO2`
- `TFT_LED = 255` (special value library, tidak mengikat ke pin PWM MCU)

Catatan:
- Kode memakai `SPI.h` dan tidak melakukan `SPI.begin(...)` custom, jadi memakai hardware SPI default ESP32 (VSPI).
- Dengan default ESP32 Dev Module:
  - `SCK = GPIO18`
  - `MOSI = GPIO23`
  - `MISO = GPIO19` (umumnya tidak wajib untuk TFT write-only, tergantung modul)

## 2. Mapping ESP32 -> TFT ILI9225

Gunakan mapping berikut untuk layout PCB:

- `ESP32 GPIO5` -> `TFT CS`
- `ESP32 GPIO4` -> `TFT RST`
- `ESP32 GPIO2` -> `TFT RS/DC`
- `ESP32 GPIO18` -> `TFT SCK/CLK`
- `ESP32 GPIO23` -> `TFT SDA/MOSI`
- `ESP32 GPIO19` -> `TFT MISO/SDO` (opsional jika modul tidak butuh readback)
- `ESP32 3V3` -> `TFT VCC`
- `ESP32 GND` -> `TFT GND`

Backlight:
- Jika modul TFT punya pin `LED`/`BL`, saat ini belum dikontrol langsung oleh MCU.
- Untuk versi sekarang, sambungkan `LED/BL` ke `3V3` (langsung atau via resistor/transistor sesuai modul).

## 3. Pin Lain yang Tidak Dipakai

Pada kode saat ini tidak ada penggunaan:
- I2C (`SDA/SCL`)
- UART tambahan
- ADC input sensor real
- GPIO relay/digital output lain

Artinya pin lain masih bebas untuk pengembangan berikutnya.

## 4. Catatan Penting untuk Desain PCB

### 4.1 Strapping/Boot Pin ESP32
Pin berikut adalah pin sensitif saat boot:
- `GPIO2` (saat ini dipakai untuk `TFT_RS/DC`)
- `GPIO5` (saat ini dipakai untuk `TFT_CS`)

Rekomendasi:
- Pastikan rangkaian TFT tidak memaksa level yang mengganggu proses boot.
- Hindari beban kuat/pull-up/pull-down eksternal yang bertentangan pada pin tersebut.
- Jika ingin desain lebih robust untuk produksi, pertimbangkan pindah `CS/DC` ke GPIO non-strapping.

### 4.2 Level Tegangan
- ESP32 adalah logika 3.3V.
- Pastikan modul ILI9225 kompatibel 3.3V logic.
- Jika modul TFT 5V logic-only, gunakan level shifter.

### 4.3 Catu Daya
- Pastikan regulator 3.3V cukup untuk ESP32 + backlight TFT.
- Tambahkan kapasitor decoupling dekat ESP32 dan konektor TFT.

## 5. Ringkasan Cepat (Untuk Net Label PCB)

- `LCD_CS` -> `GPIO5`
- `LCD_RST` -> `GPIO4`
- `LCD_DC` -> `GPIO2`
- `LCD_SCK` -> `GPIO18`
- `LCD_MOSI` -> `GPIO23`
- `LCD_MISO` -> `GPIO19` (optional)
- `LCD_VCC` -> `3V3`
- `LCD_GND` -> `GND`
- `LCD_BL` -> `3V3` (atau driver backlight eksternal)

