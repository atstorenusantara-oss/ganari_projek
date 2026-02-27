#include "dashboard_ui.h"

void drawDashboardHeader(TFT_22_ILI9225& tft) {
  tft.setFont(Terminal12x16);
  tft.drawText(15, 8, "GANARI AIR QUALITY", COLOR_CYAN);
  tft.setFont(Terminal6x8);
  tft.drawText(80, 25, "powered by: insalusi", COLOR_LIGHTCYAN);
  tft.drawRectangle(5, 35, 215, 37, COLOR_GRAY);
}

void showWifiConnectingScreen(TFT_22_ILI9225& tft, const char* ssid) {
  tft.fillRectangle(0, 45, 219, 175, COLOR_BLACK);
  tft.setFont(Terminal12x16);
  tft.drawText(10, 55, "WiFi: CONNECTING", COLOR_YELLOW);
  tft.setFont(Terminal6x8);
  tft.drawText(10, 75, "Connecting to:", COLOR_LIGHTCYAN);
  tft.drawText(10, 87, String(ssid), COLOR_LIGHTCYAN);
  tft.drawText(10, 103, "Status: starting...", COLOR_WHITE);
}

void updateWifiConnectingScreen(TFT_22_ILI9225& tft, uint8_t dotCount, const char* ssid) {
  String dots = "";
  for (uint8_t i = 0; i < dotCount; i++) {
    dots += ".";
  }
  tft.fillRectangle(10, 103, 205, 12, COLOR_BLACK);
  tft.drawText(10, 103, "Status: connecting to " + String(ssid) + dots, COLOR_WHITE);
}

void showWifiConnectedScreen(TFT_22_ILI9225& tft, const IPAddress& ip) {
  tft.fillRectangle(0, 45, 219, 175, COLOR_BLACK);
  tft.setFont(Terminal12x16);
  tft.drawText(10, 55, "WiFi: CONNECTED", COLOR_GREEN);
  tft.setFont(Terminal6x8);
  tft.drawText(10, 85, "IP: " + ip.toString(), COLOR_LIGHTCYAN);
  tft.drawText(10, 100, "Dashboard starts in 5s...", COLOR_YELLOW);
}

void showWifiCheckModeScreen(TFT_22_ILI9225& tft, const String& ssid, const String& password) {
  tft.fillRectangle(0, 45, 219, 175, COLOR_BLACK);
  tft.setFont(Terminal12x16);
  tft.drawText(10, 50, "CHECK MODE", COLOR_ORANGE);
  tft.setFont(Terminal6x8);
  tft.drawText(10, 72, "Power toggled 3x < 3s", COLOR_YELLOW);
  tft.drawText(10, 92, "SSID: " + ssid, COLOR_LIGHTCYAN);
  tft.drawText(10, 110, "PASS: " + password, COLOR_LIGHTCYAN);
  tft.drawText(10, 140, "Power cycle normal to exit", COLOR_WHITE);
}

void drawSensorValues(
  TFT_22_ILI9225& tft,
  int co2,
  float humidity,
  int pm25,
  float temp,
  bool wifiOff,
  const String& wifiIp
) {
  tft.fillRectangle(0, 45, 219, 175, COLOR_BLACK);
  tft.setFont(Terminal12x16);

  const int leftX = 10;
  int y = 48;

  tft.drawText(leftX, y, "CO2  : " + String(co2) + " ppm", COLOR_YELLOW);
  y += 28;
  tft.drawLine(8, y - 6, 212, y - 6, COLOR_DARKGREY);

  tft.drawText(leftX, y, "Humidity: " + String(humidity, 1) + " %RH", COLOR_GOLD);
  y += 28;
  tft.drawLine(8, y - 6, 212, y - 6, COLOR_DARKGREY);

  tft.drawText(leftX, y, "PM2.5: " + String(pm25) + " ug/m3", COLOR_GREENYELLOW);
  y += 28;
  tft.drawLine(8, y - 6, 212, y - 6, COLOR_DARKGREY);

  tft.drawText(leftX, y, "TEMP : " + String(temp, 1) + " C", COLOR_AZUR);

  tft.setFont(Terminal6x8);
  tft.drawText(10, 170, wifiOff ? "WiFi: OFF" : "WiFi: ON", wifiOff ? COLOR_RED : COLOR_GREEN);
  tft.drawText(90, 170, "IP: " + (wifiOff ? String("-") : wifiIp), COLOR_LIGHTCYAN);

}
