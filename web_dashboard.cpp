#include "web_dashboard.h"

void sendDashboardHtml(WebServer& server, const SensorData& sensorData) {
  String html = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>GANARI Air Quality Dashboard</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background: #111;
      color: #eee;
      margin: 0;
      padding: 20px;
      text-align: center;
    }
    h1 {
      color: #00ffff;
      margin-bottom: 5px;
    }
    .container {
      max-width: 500px;
      margin: 0 auto;
    }
    .card {
      background: #1e1e1e;
      border-radius: 12px;
      padding: 20px;
      margin: 15px 0;
      box-shadow: 0 4px 12px rgba(0,0,0,0.6);
    }
    .label {
      font-size: 1.1em;
      color: #aaa;
      margin-bottom: 6px;
    }
    .value {
      font-size: 2.8em;
      font-weight: bold;
      margin: 8px 0;
    }
    .unit {
      font-size: 0.8em;
      color: #888;
    }
    hr {
      border: 0;
      height: 1px;
      background: #444;
      margin: 25px 0;
    }
    footer {
      margin-top: 40px;
      font-size: 0.8em;
      color: #666;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>GANARI AIR QUALITY</h1>
    <p>Real-time Monitoring</p>

    <div class="card">
      <div class="label">CO2</div>
      <div class="value">)=====" + String(sensorData.co2) + R"=====(</div>
      <div class="unit">ppm</div>
    </div>

    <div class="card">
      <div class="label">O2</div>
      <div class="value">)=====" + String(sensorData.o2, 1) + R"=====(</div>
      <div class="unit">%</div>
    </div>

    <div class="card">
      <div class="label">PM2.5</div>
      <div class="value">)=====" + String(sensorData.pm25) + R"=====(</div>
      <div class="unit">ug/m3</div>
    </div>

    <div class="card">
      <div class="label">Temperature</div>
      <div class="value">)=====" + String(sensorData.temp, 1) + R"=====(</div>
      <div class="unit">C</div>
    </div>

    <hr>
    <footer>Update setiap ~3 detik | ESP32 - TFT ILI9225</footer>
  </div>

  <script>
    setTimeout(() => location.reload(), 3200);
  </script>
</body>
</html>
)=====";

  server.send(200, "text/html", html);
}
