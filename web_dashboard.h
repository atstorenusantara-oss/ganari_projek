#ifndef WEB_DASHBOARD_H
#define WEB_DASHBOARD_H

#include <Arduino.h>
#include <WebServer.h>
#include "sensor_data.h"

void sendDashboardHtml(WebServer& server, const SensorData& sensorData);
void sendWifiConfigHtml(WebServer& server, const String& currentSsid);

#endif
