#include "dashboard_ui.h"

void drawDashboardHeader(TFT_22_ILI9225& tft) {
  tft.setFont(Terminal12x16);
  tft.drawText(15, 8, "GANARI AIR QUALITY", COLOR_CYAN);
  tft.setFont(Terminal6x8);
  tft.drawText(120, 25, "Copyright jonas", COLOR_LIGHTCYAN);
  tft.drawRectangle(5, 35, 215, 37, COLOR_GRAY);
}

void showWifiConnectingScreen(TFT_22_ILI9225& tft, const char* ssid) {
  tft.fillRectangle(0, 45, 219, 175, COLOR_BLACK);
  tft.setFont(Terminal12x16);
  tft.drawText(10, 55, "WiFi: CONNECTING", COLOR_YELLOW);
  tft.setFont(Terminal6x8);
  tft.drawText(10, 75, "SSID: " + String(ssid), COLOR_LIGHTCYAN);
  tft.drawText(10, 95, "Status: starting...", COLOR_WHITE);
}

void updateWifiConnectingScreen(TFT_22_ILI9225& tft, uint8_t dotCount) {
  String dots = "";
  for (uint8_t i = 0; i < dotCount; i++) {
    dots += ".";
  }
  tft.fillRectangle(10, 95, 200, 12, COLOR_BLACK);
  tft.drawText(10, 95, "Status: connecting" + dots, COLOR_WHITE);
}

void showWifiConnectedScreen(TFT_22_ILI9225& tft, const IPAddress& ip) {
  tft.fillRectangle(0, 45, 219, 175, COLOR_BLACK);
  tft.setFont(Terminal12x16);
  tft.drawText(10, 55, "WiFi: CONNECTED", COLOR_GREEN);
  tft.setFont(Terminal6x8);
  tft.drawText(10, 85, "IP: " + ip.toString(), COLOR_LIGHTCYAN);
  tft.drawText(10, 100, "Dashboard starts in 5s...", COLOR_YELLOW);
}

void drawSensorValues(TFT_22_ILI9225& tft, int co2, float o2, int pm25, float temp) {
  tft.fillRectangle(0, 45, 219, 175, COLOR_BLACK);
  tft.setFont(Terminal12x16);

  int y = 50;
  const int labelX = 15;
  const int valueX = 100;

  tft.drawText(labelX, y, "CO2", COLOR_WHITE);
  tft.drawText(valueX, y, String(co2) + " ppm", COLOR_WHITE);
  tft.drawLine(5, y + 20, 210, y + 20, COLOR_DARKGREY);
  y += 25;

  tft.drawText(labelX, y, "O2", COLOR_WHITE);
  tft.drawText(valueX, y, String(o2, 1) + " %", COLOR_WHITE);
  tft.drawLine(5, y + 20, 210, y + 20, COLOR_DARKGREY);
  y += 25;

  tft.drawText(labelX, y, "PM2.5", COLOR_WHITE);
  tft.drawText(valueX, y, String(pm25) + " ug/m3", COLOR_WHITE);
  tft.drawLine(5, y + 20, 210, y + 20, COLOR_DARKGREY);
  y += 25;

  tft.drawText(labelX, y, "Temp", COLOR_WHITE);
  tft.drawText(valueX, y, String(temp, 1) + " C", COLOR_WHITE);
}
