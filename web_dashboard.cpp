#include "web_dashboard.h"

static String escapeHtml(const String& input) {
  String out;
  out.reserve(input.length() + 8);
  for (size_t i = 0; i < input.length(); ++i) {
    const char c = input.charAt(i);
    if (c == '&') {
      out += "&amp;";
    } else if (c == '<') {
      out += "&lt;";
    } else if (c == '>') {
      out += "&gt;";
    } else if (c == '"') {
      out += "&quot;";
    } else if (c == '\'') {
      out += "&#39;";
    } else {
      out += c;
    }
  }
  return out;
}

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
    .btn-link {
      display: inline-block;
      margin-top: 12px;
      text-decoration: none;
      border: 0;
      border-radius: 8px;
      padding: 10px 14px;
      background: #00b3b3;
      color: #041414;
      font-weight: bold;
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

    <div class="card">
      <div class="label">VOC</div>
      <div class="value">)=====" + String(sensorData.VOC, 1) + R"=====(</div>
      <div class="unit">-</div>
    </div>

    <a class="btn-link" href="/wifi">WiFi Settings</a>

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

void sendWifiConfigHtml(WebServer& server, const String& currentSsid) {
  const String safeSsid = escapeHtml(currentSsid);
  String html = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>WiFi Settings</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background: #111;
      color: #eee;
      margin: 0;
      padding: 20px;
    }
    .container {
      max-width: 500px;
      margin: 0 auto;
    }
    .card {
      background: #1e1e1e;
      border-radius: 12px;
      padding: 20px;
      box-shadow: 0 4px 12px rgba(0,0,0,0.6);
    }
    .title {
      margin: 0 0 6px;
      color: #00ffff;
      font-size: 1.3em;
    }
    .hint {
      color: #888;
      font-size: 0.9em;
      margin: 0 0 16px;
    }
    label {
      display: block;
      margin: 10px 0 6px;
      color: #bbb;
    }
    input {
      width: 100%;
      box-sizing: border-box;
      border: 1px solid #333;
      background: #0f0f0f;
      color: #eee;
      padding: 10px;
      border-radius: 8px;
      font-size: 0.95em;
    }
    button {
      margin-top: 14px;
      width: 100%;
      border: 0;
      border-radius: 8px;
      padding: 10px 12px;
      background: #00b3b3;
      color: #041414;
      font-weight: bold;
      cursor: pointer;
    }
    .links {
      margin-top: 14px;
      text-align: center;
    }
    a {
      color: #9fd;
      text-decoration: none;
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="card">
      <h1 class="title">WiFi Settings</h1>
      <p class="hint">SSID aktif: )=====" + safeSsid + R"=====(</p>
      <form method="POST" action="/wifi-save">
        <label for="ssid">SSID</label>
        <input id="ssid" name="ssid" type="text" required>
        <label for="password">Password</label>
        <input id="password" name="password" type="password">
        <button type="submit">Update WiFi</button>
      </form>
      <p class="hint">Board akan reconnect otomatis setelah disimpan.</p>
      <div class="links"><a href="/">Kembali ke dashboard</a></div>
    </div>
  </div>
</body>
</html>
)=====";

  server.send(200, "text/html", html);
}
