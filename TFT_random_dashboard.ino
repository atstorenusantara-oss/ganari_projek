#include <Arduino.h>
#include <SPI.h>
#include <TFT_22_ILI9225.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoOTA.h>
#include <Update.h>

#include "dashboard_ui.h"
#include "sensor_data.h"
#include "web_dashboard.h"

const char* ssid = "TUBIS43LT2";
const char* password = "12345678";

#define TFT_CS   5
#define TFT_RST  4
#define TFT_RS   2
#define TFT_LED  255

int beta = 0;

TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_LED);
WebServer server(80);

SensorData sensorData = {450, 20.8, 12, 27.5};
const char* otaHttpUser = "admin";
const char* otaHttpPass = "admin123";
wl_status_t lastWifiStatus = WL_IDLE_STATUS;
unsigned long lastWifiRetryMs = 0;
unsigned long lastWifiAnimMs = 0;
uint8_t wifiDotCount = 0;
const unsigned long wifiRetryIntervalMs = 5000;
const unsigned long wifiAnimIntervalMs = 500;

void handleRoot() {
  Serial.println("HTTP GET /");
  sendDashboardHtml(server, sensorData);
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

  tft.begin();
  tft.setOrientation(1);
  tft.clear();
  drawDashboardHeader(tft);

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  showWifiConnectingScreen(tft, ssid);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi ");
  uint8_t dotCount = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

    dotCount = (dotCount + 1) % 4;
    updateWifiConnectingScreen(tft, dotCount);
  }

  Serial.println("\nConnected!");
  Serial.print("IP address: http://");
  Serial.println(WiFi.localIP());

  setupOta();
  Serial.print("OTA host: ");
  Serial.println("ganari-dashboard.local");

  showWifiConnectedScreen(tft, WiFi.localIP());
  delay(5000);

  server.on("/", handleRoot);
  server.on("/ping", HTTP_GET, handlePing);
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

  drawDashboardHeader(tft);
  lastWifiStatus = WL_CONNECTED;
}

void loop() {
  static unsigned long lastUpdateMs = 0;
  const unsigned long updateIntervalMs = 3000;

  const wl_status_t wifiStatus = WiFi.status();
  if (wifiStatus != WL_CONNECTED) {
    if (lastWifiStatus == WL_CONNECTED) {
      Serial.println("WiFi disconnected. Trying to reconnect...");
      showWifiConnectingScreen(tft, ssid);
    }

    if (millis() - lastWifiAnimMs >= wifiAnimIntervalMs) {
      lastWifiAnimMs = millis();
      wifiDotCount = (wifiDotCount + 1) % 4;
      updateWifiConnectingScreen(tft, wifiDotCount);
    }

    if (millis() - lastWifiRetryMs >= wifiRetryIntervalMs) {
      lastWifiRetryMs = millis();
      WiFi.reconnect();
      Serial.println("WiFi reconnect attempt sent");
    }

    lastWifiStatus = wifiStatus;
    return;
  }

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

  if (millis() - lastUpdateMs < updateIntervalMs) {
    return;
  }
  lastUpdateMs = millis();

  updateSensorDataRandom(sensorData);
  drawSensorValues(tft, sensorData.co2, sensorData.o2, sensorData.pm25, sensorData.temp);
  Serial.println("UALUBUN --- "+ String(beta++ * 100));
}
