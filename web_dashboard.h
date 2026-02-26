#ifndef WEB_DASHBOARD_H
#define WEB_DASHBOARD_H

#include <WebServer.h>
#include "sensor_data.h"

void sendDashboardHtml(WebServer& server, const SensorData& sensorData);

#endif
