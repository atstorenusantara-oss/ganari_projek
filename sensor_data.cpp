#include <Arduino.h>
#include "sensor_data.h"

namespace {
HardwareSerial sensorSerial(2);

static const int RX2_PIN = 16;
static const int TX2_PIN = 17;

uint8_t frameBuf[64];
size_t framePos = 0;

struct ParsedData {
  uint16_t co2 = 0;
  uint16_t voc = 0;
  float humidity = 0;
  float temperature = 0;
  uint16_t pm25 = 0;
  int score = -999;
};

uint8_t calcChecksum(const uint8_t* buf, size_t lenWithoutCs) {
  uint8_t sum = 0;
  for (size_t i = 0; i < lenWithoutCs; i++) {
    sum += buf[i];
  }
  return static_cast<uint8_t>(~sum + 1);
}

uint16_t readU16(const uint8_t* f, size_t idx, bool littleEndian) {
  if (littleEndian) {
    return (uint16_t(f[idx + 1]) << 8) | f[idx];
  }
  return (uint16_t(f[idx]) << 8) | f[idx + 1];
}

int scoreData(const ParsedData& d) {
  int s = 0;
  if (d.co2 <= 5000) s += 3;
  if (d.co2 >= 250 && d.co2 <= 2500) s += 2;
  if (d.humidity >= 0 && d.humidity <= 100) s += 3;
  if (d.humidity >= 20 && d.humidity <= 90) s += 1;
  if (d.temperature >= -20 && d.temperature <= 80) s += 3;
  if (d.temperature >= 10 && d.temperature <= 45) s += 1;
  if (d.pm25 <= 2000) s += 2;
  if (d.pm25 <= 500) s += 1;
  if (d.voc <= 5000) s += 1;
  return s;
}

bool tryDecode(const uint8_t* f, uint8_t len, bool hasStatus, bool littleEndian, ParsedData& out) {
  const size_t base = hasStatus ? 4 : 3;
  if (base + 8 > len) {
    return false;
  }

  ParsedData d;
  d.co2 = readU16(f, base + 0, littleEndian);
  d.voc = readU16(f, base + 2, littleEndian);

  const uint16_t humRaw = readU16(f, base + 4, littleEndian);
  const uint16_t tempRaw = readU16(f, base + 6, littleEndian);
  d.humidity = humRaw / 10.0f;
  d.temperature = (tempRaw - 500) / 10.0f;
  d.pm25 = readU16(f, base + 8, littleEndian);
  d.score = scoreData(d);

  out = d;
  return true;
}

bool parseFrameToSensorData(const uint8_t* frame, size_t totalLen, SensorData& out) {
  (void)totalLen;
  const uint8_t len = frame[1];
  const uint8_t cmd = frame[2];

  if (cmd != 0x01 && cmd != 0x02) {
    return false;
  }
  if (len < 0x0B) {
    return false;
  }

  ParsedData best;
  bool found = false;

  ParsedData cand;
  if (tryDecode(frame, len, false, false, cand)) {
    best = cand;
    found = true;
  }
  if (tryDecode(frame, len, true, false, cand)) {
    if (!found || cand.score > best.score) best = cand;
    found = true;
  }
  if (tryDecode(frame, len, false, true, cand)) {
    if (!found || cand.score > best.score) best = cand;
    found = true;
  }
  if (tryDecode(frame, len, true, true, cand)) {
    if (!found || cand.score > best.score) best = cand;
    found = true;
  }

  if (!found) {
    return false;
  }

  out.co2 = best.co2;
  out.voc = best.voc;
  out.humidity = best.humidity;
  out.pm25 = best.pm25;
  out.temp = best.temperature;
  return true;
}

void sendQuery(uint8_t cmd) {
  uint8_t pkt[5] = {0x11, 0x02, cmd, 0x00, 0x00};
  pkt[4] = calcChecksum(pkt, 4);
  sensorSerial.write(pkt, sizeof(pkt));
}
}  // namespace

void initSensorReader() {
  sensorSerial.begin(9600, SERIAL_8N1, RX2_PIN, TX2_PIN);
  delay(200);
  sendQuery(0x01);
  Serial.println("ZPHS01C reader initialized (UART2 9600)");
}

bool updateSensorDataFromSensor(SensorData& data) {
  bool updated = false;

  while (sensorSerial.available()) {
    const uint8_t b = sensorSerial.read();

    if (framePos == 0) {
      if (b != 0x16) continue;
      frameBuf[framePos++] = b;
      continue;
    }

    frameBuf[framePos++] = b;

    if (framePos >= 2) {
      const size_t expected = frameBuf[1] + 3;

      if (expected > sizeof(frameBuf)) {
        framePos = 0;
        continue;
      }

      if (framePos == expected) {
        const uint8_t cs = calcChecksum(frameBuf, expected - 1);
        const uint8_t rxCs = frameBuf[expected - 1];

        if (cs == rxCs) {
          SensorData parsed = data;
          if (parseFrameToSensorData(frameBuf, expected, parsed)) {
            data = parsed;
            updated = true;
          }
        }

        framePos = 0;
      }
    }

    if (framePos >= sizeof(frameBuf)) {
      framePos = 0;
    }
  }

  return updated;
}
