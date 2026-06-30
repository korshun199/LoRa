#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

const char* CALLSIGN = "BRATAN";
const char* AP_SSID = "LORA-BRATAN";
const char* AP_PASS = "12345678";

IPAddress local_ip(192, 168, 4, 10);
IPAddress gateway(192, 168, 4, 10);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);

String htmlPage() {
  String html = R"HTML(
<!doctype html>
<html lang="ru">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>LoRa Link Test</title>
  <style>
    body {
      margin: 0;
      font-family: system-ui, Arial, sans-serif;
      background: #10131a;
      color: #e8eefc;
    }
    .wrap {
      max-width: 900px;
      margin: 0 auto;
      padding: 18px;
    }
    .card {
      background: #181d29;
      border: 1px solid #2a3142;
      border-radius: 14px;
      padding: 16px;
      margin-bottom: 14px;
      box-shadow: 0 8px 24px rgba(0,0,0,.25);
    }
    h1 {
      margin: 0 0 12px;
      font-size: 24px;
    }
    .big {
      font-size: 22px;
      font-weight: 700;
    }
    .muted {
      color: #9aa7bd;
    }
    table {
      width: 100%;
      border-collapse: collapse;
      margin-top: 8px;
    }
    th, td {
      padding: 10px;
      border-bottom: 1px solid #2a3142;
      text-align: left;
    }
    th {
      color: #9aa7bd;
      font-weight: 600;
    }
    .ok {
      color: #8cffb3;
    }
  </style>
</head>
<body>
  <div class="wrap">
    <h1>LoRa Link Test Web</h1>

    <div class="card">
      <div class="muted">Моё устройство</div>
      <div class="big" id="callsign">...</div>
      <p>IP: <span id="ip">...</span></p>
      <p>Uptime: <span id="uptime">...</span> сек</p>
      <p>Status: <span class="ok">WEB ONLINE</span></p>
    </div>

    <div class="card">
      <div class="muted">Соседи LoRa</div>
      <table>
        <thead>
          <tr>
            <th>Позывной</th>
            <th>IP</th>
            <th>RSSI</th>
            <th>SNR</th>
            <th>Качество</th>
          </tr>
        </thead>
        <tbody id="peers">
          <tr>
            <td colspan="5" class="muted">LoRa пока не включена</td>
          </tr>
        </tbody>
      </table>
    </div>
  </div>

<script>
async function updateStatus() {
  try {
    const r = await fetch('/api/status');
    const s = await r.json();

    document.getElementById('callsign').textContent = s.callsign;
    document.getElementById('ip').textContent = s.ip;
    document.getElementById('uptime').textContent = Math.floor(s.uptime_ms / 1000);
  } catch(e) {
    console.log(e);
  }
}

setInterval(updateStatus, 1000);
updateStatus();
</script>
</body>
</html>
)HTML";
  return html;
}

void handleRoot() {
  server.send(200, "text/html; charset=utf-8", htmlPage());
}

void handleStatus() {
  StaticJsonDocument<512> doc;

  doc["callsign"] = CALLSIGN;
  doc["ip"] = WiFi.softAPIP().toString();
  doc["uptime_ms"] = millis();
  doc["wifi_mode"] = "AP";
  doc["lora"] = "not_ready";

  String out;
  serializeJson(doc, out);
  server.send(200, "application/json; charset=utf-8", out);
}

void handlePeers() {
  server.send(200, "application/json; charset=utf-8", "[]");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("=== LoRa Link Test Web ===");

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(AP_SSID, AP_PASS);

  Serial.print("AP SSID: ");
  Serial.println(AP_SSID);
  Serial.print("AP PASS: ");
  Serial.println(AP_PASS);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/api/status", handleStatus);
  server.on("/api/peers", handlePeers);
  server.begin();

  Serial.println("Web server started");
}

void loop() {
  server.handleClient();
}
