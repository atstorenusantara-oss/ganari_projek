#ifndef DASHBOARD_UI_H
#define DASHBOARD_UI_H

#include <Arduino.h>
#include <TFT_22_ILI9225.h>
#include <WiFi.h>

void drawDashboardHeader(TFT_22_ILI9225& tft);
void showWifiConnectingScreen(TFT_22_ILI9225& tft, const char* ssid);
void updateWifiConnectingScreen(TFT_22_ILI9225& tft, uint8_t dotCount);
void showWifiConnectedScreen(TFT_22_ILI9225& tft, const IPAddress& ip);
void drawSensorValues(TFT_22_ILI9225& tft, int co2, float o2, int pm25, float temp);

#endif
