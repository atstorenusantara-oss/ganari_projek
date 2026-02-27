#include <Arduino.h>

HardwareSerial SensorSerial(2);  // UART2

static const int RX2_PIN = 16;
static const int TX2_PIN = 17;

uint8_t frameBuf[64];
size_t framePos = 0;

struct ParsedData {
  uint16_t co2 = 0;
  uint16_t vocOrCh2o = 0;
  float humidity = 0;
  float temperature = 0;
  uint16_t pm25 = 0;
  bool hasPmExt = false;
  uint16_t pm10 = 0;
  uint16_t pm1_0 = 0;
  int score = -999;
  const char *layout = "unknown";
};

uint8_t calcChecksum(const uint8_t *buf, size_t lenWithoutCs) {
  uint8_t sum = 0;
  for (size_t i = 0; i < lenWithoutCs; i++) {
    sum += buf[i];
  }
  return static_cast<uint8_t>(~sum + 1);
}

uint16_t readU16(const uint8_t *f, size_t idx, bool littleEndian) {
  if (littleEndian) {
    return (uint16_t(f[idx + 1]) << 8) | f[idx];
  }
  return (uint16_t(f[idx]) << 8) | f[idx + 1];
}

int scoreData(const ParsedData &d) {
  int s = 0;

  if (d.co2 <= 5000) s += 3;
  if (d.co2 >= 250 && d.co2 <= 2500) s += 2;

  if (d.humidity >= 0 && d.humidity <= 100) s += 3;
  if (d.humidity >= 20 && d.humidity <= 90) s += 1;

  if (d.temperature >= -20 && d.temperature <= 80) s += 3;
  if (d.temperature >= 10 && d.temperature <= 45) s += 1;

  if (d.pm25 <= 2000) s += 2;
  if (d.pm25 <= 500) s += 1;

  if (d.vocOrCh2o <= 5000) s += 1;

  if (d.hasPmExt) {
    if (d.pm10 <= 3000) s += 1;
    if (d.pm1_0 <= 3000) s += 1;
  }

  return s;
}

bool tryDecode(const uint8_t *f, uint8_t len, uint8_t cmd, bool hasStatus, bool littleEndian,
               const char *layout, ParsedData &out) {
  const size_t base = hasStatus ? 4 : 3;

  // data indices must be inside [3 .. len+1]
  if (base + 8 > len) {
    return false;
  }

  ParsedData d;
  d.layout = layout;

  d.co2 = readU16(f, base + 0, littleEndian);
  d.vocOrCh2o = readU16(f, base + 2, littleEndian);

  const uint16_t humRaw = readU16(f, base + 4, littleEndian);
  const uint16_t tempRaw = readU16(f, base + 6, littleEndian);
  d.humidity = humRaw / 10.0f;
  d.temperature = (tempRaw - 500) / 10.0f;
  d.pm25 = readU16(f, base + 8, littleEndian);

  if (cmd == 0x02 && (base + 12 <= len)) {
    d.hasPmExt = true;
    d.pm10 = readU16(f, base + 10, littleEndian);
    d.pm1_0 = readU16(f, base + 12, littleEndian);
  }

  d.score = scoreData(d);
  out = d;
  return true;
}

void printHexFrame(const uint8_t *buf, size_t n) {
  Serial.print("RX: ");
  for (size_t i = 0; i < n; i++) {
    if (buf[i] < 0x10) Serial.print('0');
    Serial.print(buf[i], HEX);
    Serial.print(' ');
  }
  Serial.println();
}

void parseFrame(const uint8_t *f, size_t totalLen) {
  (void)totalLen;

  const uint8_t len = f[1];
  const uint8_t cmd = f[2];

  if (cmd != 0x01 && cmd != 0x02) {
    Serial.printf("CMD tidak didukung: 0x%02X\n", cmd);
    return;
  }

  if (len < 0x0B) {
    Serial.println("LEN terlalu kecil.");
    return;
  }

  ParsedData best;
  bool found = false;

  ParsedData cand;
  if (tryDecode(f, len, cmd, false, false, "no-status big-endian", cand)) {
    best = cand;
    found = true;
  }
  if (tryDecode(f, len, cmd, true, false, "with-status big-endian", cand)) {
    if (!found || cand.score > best.score) best = cand;
    found = true;
  }
  if (tryDecode(f, len, cmd, false, true, "no-status little-endian", cand)) {
    if (!found || cand.score > best.score) best = cand;
    found = true;
  }
  if (tryDecode(f, len, cmd, true, true, "with-status little-endian", cand)) {
    if (!found || cand.score > best.score) best = cand;
    found = true;
  }

  if (!found) {
    Serial.println("Gagal decode frame (layout tidak cocok LEN).\n");
    return;
  }

  Serial.println("----- ZPHS01C -----");
  Serial.printf("CMD      : 0x%02X\n", cmd);
  Serial.printf("Layout   : %s (score=%d)\n", best.layout, best.score);
  Serial.printf("CO2      : %u ppm\n", best.co2);
  Serial.printf("VOC/CH2O : %u\n", best.vocOrCh2o);
  Serial.printf("Humidity : %.1f %%RH\n", best.humidity);
  Serial.printf("Temp     : %.1f C\n", best.temperature);
  Serial.printf("PM2.5    : %u ug/m3\n", best.pm25);

  if (best.hasPmExt) {
    Serial.printf("PM10     : %u ug/m3\n", best.pm10);
    Serial.printf("PM1.0    : %u ug/m3\n", best.pm1_0);
  }

  if (best.score < 6) {
    Serial.println("Warning  : hasil decode masih kurang meyakinkan, cek frame tambahan.");
  }

  Serial.println("-------------------");
}

void sendQuery(uint8_t cmd) {
  uint8_t pkt[5] = {0x11, 0x02, cmd, 0x00, 0x00};
  pkt[4] = calcChecksum(pkt, 4);

  SensorSerial.write(pkt, sizeof(pkt));

  Serial.print("TX: ");
  for (uint8_t b : pkt) {
    if (b < 0x10) Serial.print('0');
    Serial.print(b, HEX);
    Serial.print(' ');
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  SensorSerial.begin(9600, SERIAL_8N1, RX2_PIN, TX2_PIN);

  delay(200);
  Serial.println("Start ZPHS01C UART...");

  sendQuery(0x01);  // Active upload
  // sendQuery(0x02);  // Q&A mode (PM10 + PM1.0)
}

void loop() {
  while (SensorSerial.available()) {
    const uint8_t b = SensorSerial.read();

    if (framePos == 0) {
      if (b != 0x16) continue;
      frameBuf[framePos++] = b;
      continue;
    }

    frameBuf[framePos++] = b;

    if (framePos >= 2) {
      const size_t expected = frameBuf[1] + 3;  // HEAD + LEN + (CMD+DATA) + CS

      if (expected > sizeof(frameBuf)) {
        Serial.println("Frame terlalu panjang, reset buffer.");
        framePos = 0;
        continue;
      }

      if (framePos == expected) {
        printHexFrame(frameBuf, expected);

        const uint8_t cs = calcChecksum(frameBuf, expected - 1);
        const uint8_t rxCs = frameBuf[expected - 1];

        if (cs == rxCs) {
          parseFrame(frameBuf, expected);
        } else {
          Serial.printf("Checksum salah. calc=0x%02X rx=0x%02X\n", cs, rxCs);
        }

        framePos = 0;
      }
    }

    if (framePos >= sizeof(frameBuf)) {
      framePos = 0;
    }
  }
}
