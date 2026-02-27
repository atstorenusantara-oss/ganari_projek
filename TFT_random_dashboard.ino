#include <Arduino.h>
#include <SPI.h>
#include <TFT_22_ILI9225.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoOTA.h>
#include <Update.h>
#include <EEPROM.h>

#include "dashboard_ui.h"
#include "sensor_data.h"
#include "web_dashboard.h"

const char* defaultSsid = "TUBIS43LT2";
const char* defaultPassword = "12345678";

#define TFT_CS   5
#define TFT_RST  4
#define TFT_RS   2
#define TFT_LED  255

TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_LED);
WebServer server(80);

SensorData sensorData = {450, 100, 55.0, 12, 27.5};
String wifiSsid;
String wifiPassword;
const char* otaHttpUser = "admin";
const char* otaHttpPass = "admin123";
wl_status_t lastWifiStatus = WL_IDLE_STATUS;
unsigned long lastWifiRetryMs = 0;
unsigned long lastWifiAnimMs = 0;
uint8_t wifiDotCount = 0;
const unsigned long wifiRetryIntervalMs = 5000;
const unsigned long wifiAnimIntervalMs = 500;
const unsigned long wifiDisableTimeoutMs = 60000;
const int EEPROM_SIZE = 128;
const int EEPROM_SSID_ADDR = 0;
const int EEPROM_SSID_LEN = 32;
const int EEPROM_PASS_ADDR = 32;
const int EEPROM_PASS_LEN = 64;
const int EEPROM_BOOT_TOGGLE_ADDR = 120;
const uint8_t CHECK_MODE_THRESHOLD = 3;
const unsigned long POWER_TOGGLE_WINDOW_MS = 3000;
unsigned long bootStartMs = 0;
bool checkModeActive = false;
bool bootToggleWindowCleared = false;
bool wifiDisabledDueTimeout = false;
unsigned long wifiDisconnectedSinceMs = 0;

static String readEepromString(int addr, int maxLen) {
  char buf[65];
  int copyLen = maxLen;
  if (copyLen > 64) {
    copyLen = 64;
  }
  if (EEPROM.read(addr) == 0xFF) {
    return String();
  }
  for (int i = 0; i < copyLen; ++i) {
    const uint8_t raw = EEPROM.read(addr + i);
    if (raw == 0xFF) {
      buf[i] = '\0';
      break;
    }
    buf[i] = static_cast<char>(raw);
  }
  buf[copyLen] = '\0';
  return String(buf);
}

static void writeEepromString(int addr, int maxLen, const String& value) {
  for (int i = 0; i < maxLen; ++i) {
    char c = '\0';
    if (i < value.length()) {
      c = value.charAt(i);
    }
    EEPROM.write(addr + i, static_cast<uint8_t>(c));
  }
}

static uint8_t readBootToggleCount() {
  const uint8_t raw = EEPROM.read(EEPROM_BOOT_TOGGLE_ADDR);
  return (raw == 0xFF) ? 0 : raw;
}

static void writeBootToggleCount(uint8_t count) {
  EEPROM.write(EEPROM_BOOT_TOGGLE_ADDR, count);
  EEPROM.commit();
}

static void clearBootToggleWindowIfNeeded() {
  if (bootToggleWindowCleared || checkModeActive) {
    return;
  }
  if (millis() - bootStartMs >= POWER_TOGGLE_WINDOW_MS) {
    writeBootToggleCount(0);
    bootToggleWindowCleared = true;
    Serial.println("Rapid power-toggle counter cleared");
  }
}

static void disableWifiDueToTimeout(const char* reason) {
  if (wifiDisabledDueTimeout) {
    return;
  }

  wifiDisabledDueTimeout = true;
  WiFi.disconnect(false, false);
  WiFi.mode(WIFI_OFF);
  lastWifiStatus = WL_DISCONNECTED;
  Serial.printf("WiFi disabled: %s\n", reason);
}

void handleRoot() {
  Serial.println("HTTP GET /");
  sendDashboardHtml(server, sensorData);
}

void handleWifiPage() {
  Serial.println("HTTP GET /wifi");
  sendWifiConfigHtml(server, wifiSsid);
}

void handleWifiSave() {
  Serial.println("HTTP POST /wifi-save");

  if (!server.hasArg("ssid") || !server.hasArg("password")) {
    server.send(400, "text/plain", "Missing ssid/password");
    return;
  }

  String newSsid = server.arg("ssid");
  String newPassword = server.arg("password");
  newSsid.trim();
  newPassword.trim();

  if (newSsid.length() == 0) {
    server.send(400, "text/plain", "SSID must not be empty");
    return;
  }

  wifiSsid = newSsid;
  wifiPassword = newPassword;

  writeEepromString(EEPROM_SSID_ADDR, EEPROM_SSID_LEN, wifiSsid);
  writeEepromString(EEPROM_PASS_ADDR, EEPROM_PASS_LEN, wifiPassword);
  EEPROM.commit();

  String html =
    "<!doctype html><html><head><meta charset='utf-8'>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>WiFi Updated</title></head><body>"
    "<h3>WiFi credential tersimpan.</h3>"
    "<p>Board sedang reconnect ke SSID baru: <b>" + wifiSsid + "</b></p>"
    "<p><a href='/wifi'>Kembali ke WiFi settings</a></p>"
    "</body></html>";
  server.send(200, "text/html", html);

  Serial.print("WiFi credentials updated. Reconnecting to: ");
  Serial.println(wifiSsid);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  showWifiConnectingScreen(tft, wifiSsid.c_str());
  WiFi.disconnect(false, false);
  delay(150);
  WiFi.begin(wifiSsid.c_str(), wifiPassword.c_str());
  wifiDisabledDueTimeout = false;
  wifiDisconnectedSinceMs = millis();
  lastWifiStatus = WL_DISCONNECTED;
  lastWifiRetryMs = millis();
  lastWifiAnimMs = millis();
  wifiDotCount = 0;
}

bool ensureUpdateAuth() {
  if (server.authenticate(otaHttpUser, otaHttpPass)) {
    return true;
  }
  Serial.println("HTTP auth failed for /update");
  server.requestAuthentication();
  return false;
}

void handlePing() {
  server.send(200, "text/plain", "pong");
}

void handleFirmwareUpdatePage() {
  Serial.println("HTTP GET /update");
  if (!ensureUpdateAuth()) {
    return;
  }

  const char* html =
    "<!doctype html><html><head><meta charset='utf-8'>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>ESP32 Firmware Update</title></head><body>"
    "<h2>ESP32 Firmware Update</h2>"
    "<form method='POST' action='/update' enctype='multipart/form-data'>"
    "<input type='file' name='firmware' accept='.bin' required>"
    "<button type='submit'>Upload</button></form>"
    "<p>Gunakan file firmware .bin dari folder .pio/build/esp32dev/</p>"
    "</body></html>";

  server.send(200, "text/html", html);
}

void handleFirmwareUpload() {
  HTTPUpload& upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    Serial.printf("HTTP OTA start: %s\n", upload.filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) {
      Serial.printf("HTTP OTA success: %u bytes\n", upload.totalSize);
    } else {
      Update.printError(Serial);
    }
  }
}

void handleFirmwareUpdateResult() {
  Serial.println("HTTP POST /update");
  if (!ensureUpdateAuth()) {
    return;
  }

  const bool success = !Update.hasError();
  server.send(200, "text/plain", success ? "Update OK. Rebooting..." : "Update failed.");
  delay(300);
  if (success) {
    ESP.restart();
  }
}

void setupOta() {
  ArduinoOTA.setHostname("ganari-dashboard");

  ArduinoOTA.onStart([]() {
    const String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
    Serial.println("OTA start: " + type);
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA end");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA progress: %u%%\r", (progress * 100U) / total);
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA error[%u]\n", error);
  });

  ArduinoOTA.begin();
  Serial.println("OTA ready");
  
}

void setup() {
  Serial.begin(115200);
  delay(300);
  bootStartMs = millis();

  tft.begin();
  tft.setOrientation(3);
  tft.clear();
  drawDashboardHeader(tft);
  initSensorReader();

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  EEPROM.begin(EEPROM_SIZE);
  wifiSsid = readEepromString(EEPROM_SSID_ADDR, EEPROM_SSID_LEN);
  wifiPassword = readEepromString(EEPROM_PASS_ADDR, EEPROM_PASS_LEN);
  if (wifiSsid.length() == 0) {
    wifiSsid = defaultSsid;
    wifiPassword = defaultPassword;
  }

  uint8_t toggleCount = readBootToggleCount();
  if (toggleCount < 255) {
    toggleCount++;
  }
  writeBootToggleCount(toggleCount);
  Serial.printf("Rapid power-toggle count: %u\n", toggleCount);
  if (toggleCount >= CHECK_MODE_THRESHOLD) {
    checkModeActive = true;
    writeBootToggleCount(0);
    showWifiCheckModeScreen(tft, wifiSsid, wifiPassword);
    Serial.println("CHECK MODE active (3x power toggle detected)");
    return;
  }

  showWifiConnectingScreen(tft, wifiSsid.c_str());
  WiFi.begin(wifiSsid.c_str(), wifiPassword.c_str());

  Serial.print("Connecting to WiFi ");
  const unsigned long connectStartMs = millis();
  uint8_t dotCount = 0;
  while (WiFi.status() != WL_CONNECTED) {
    clearBootToggleWindowIfNeeded();
    if (millis() - connectStartMs >= wifiDisableTimeoutMs) {
      disableWifiDueToTimeout("startup connect timeout > 60s");
      break;
    }
    delay(500);
    Serial.print(".");

    dotCount = (dotCount + 1) % 4;
    updateWifiConnectingScreen(tft, dotCount, wifiSsid.c_str());
  }

  if (!wifiDisabledDueTimeout && WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected!");
    Serial.print("IP address: http://");
    Serial.println(WiFi.localIP());

    setupOta();
    Serial.print("OTA host: ");
    Serial.println("ganari-dashboard.local");

    showWifiConnectedScreen(tft, WiFi.localIP());
    delay(5000);

    server.on("/", handleRoot);
    server.on("/wifi", HTTP_GET, handleWifiPage);
    server.on("/ping", HTTP_GET, handlePing);
    server.on("/wifi-save", HTTP_POST, handleWifiSave);
    server.on("/update", HTTP_GET, handleFirmwareUpdatePage);
    server.on("/update", HTTP_POST, handleFirmwareUpdateResult, handleFirmwareUpload);
    server.onNotFound([]() {
      Serial.print("HTTP 404: ");
      Serial.println(server.uri());
      server.send(404, "text/plain", "Not found");
    });
    server.begin();
    Serial.println("HTTP server started");
    Serial.print("HTTP OTA URL: http://");
    Serial.print(WiFi.localIP());
    Serial.println("/update");
  } else {
    Serial.println("\nWiFi OFF after startup timeout (60s).");
  }

  drawDashboardHeader(tft);
  lastWifiStatus = wifiDisabledDueTimeout ? WL_DISCONNECTED : WL_CONNECTED;
}

void loop() {
  if (checkModeActive) {
    return;
  }

  updateSensorDataFromSensor(sensorData);

  clearBootToggleWindowIfNeeded();

  static unsigned long lastUpdateMs = 0;
  const unsigned long updateIntervalMs = 3000;

  if (!wifiDisabledDueTimeout) {
    const wl_status_t wifiStatus = WiFi.status();
    if (wifiStatus != WL_CONNECTED) {
      if (lastWifiStatus == WL_CONNECTED) {
        Serial.println("WiFi disconnected. Trying to reconnect...");
        showWifiConnectingScreen(tft, wifiSsid.c_str());
        wifiDisconnectedSinceMs = millis();
      } else if (wifiDisconnectedSinceMs == 0) {
        wifiDisconnectedSinceMs = millis();
      }

      if (millis() - lastWifiAnimMs >= wifiAnimIntervalMs) {
        lastWifiAnimMs = millis();
        wifiDotCount = (wifiDotCount + 1) % 4;
        updateWifiConnectingScreen(tft, wifiDotCount, wifiSsid.c_str());
      }

      if (millis() - lastWifiRetryMs >= wifiRetryIntervalMs) {
        lastWifiRetryMs = millis();
        WiFi.reconnect();
        Serial.println("WiFi reconnect attempt sent");
      }

      if (wifiDisconnectedSinceMs != 0 && millis() - wifiDisconnectedSinceMs >= wifiDisableTimeoutMs) {
        disableWifiDueToTimeout("reconnect timeout > 60s");
      } else {
        lastWifiStatus = wifiStatus;
        return;
      }
    } else {
      wifiDisconnectedSinceMs = 0;
      if (lastWifiStatus != WL_CONNECTED) {
        Serial.print("WiFi reconnected. IP: http://");
        Serial.println(WiFi.localIP());
        showWifiConnectedScreen(tft, WiFi.localIP());
        delay(1200);
        drawDashboardHeader(tft);
      }
      lastWifiStatus = WL_CONNECTED;
      ArduinoOTA.handle();
      server.handleClient();
    }
  }

  if (millis() - lastUpdateMs < updateIntervalMs) {
    return;
  }
  lastUpdateMs = millis();

  const String wifiIpText = wifiDisabledDueTimeout ? "-" : WiFi.localIP().toString();
  drawSensorValues(
    tft,
    sensorData.co2,
    sensorData.humidity,
    sensorData.pm25,
    sensorData.temp,
    wifiDisabledDueTimeout,
    wifiIpText
  );
  Serial.printf(
    "Sensor -> CO2: %d ppm | VOC: %d | RH: %.1f %% | PM2.5: %d ug/m3 | Temp: %.1f C\n",
    sensorData.co2,
    sensorData.voc,
    sensorData.humidity,
    sensorData.pm25,
    sensorData.temp
  );
}
