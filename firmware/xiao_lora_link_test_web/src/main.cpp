#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <RadioLib.h>

// ===== Device defaults =====
const char* CALLSIGN = "BRATAN";
const char* AP_SSID = "LORA-BRATAN";
const char* AP_PASS = "12345678";

IPAddress local_ip(192, 168, 4, 10);
IPAddress gateway(192, 168, 4, 10);
IPAddress subnet(255, 255, 255, 0);

// ===== XIAO ESP32S3 + Wio SX1262 real pin map =====
// Confirmed working pin map:
// DIO1=D0(1), BUSY=D1(2), NRST=D2(3), NSS=D3(4), ANT_SW=D4(5)
// SCK=D8(7), MISO=D9(8), MOSI=D10(9)

#define LORA_DIO1   D0
#define LORA_BUSY   D1
#define LORA_RST    D2
#define LORA_NSS    D3
#define LORA_ANT_SW D4
#define LORA_SCK    D8
#define LORA_MISO   D9
#define LORA_MOSI   D10

// Частоту потом сделаем настраиваемой.
// Пока ставим тестовую, как контрольную точку.
float loraFreqMhz = 868.0;

SPIClass loraSPI(FSPI);
SX1262 radio = new Module(
  LORA_NSS,
  LORA_DIO1,
  LORA_RST,
  LORA_BUSY,
  loraSPI
);

WebServer server(80);

bool loraOk = false;
int loraBeginCode = 999;

unsigned long lastBeaconMs = 0;
const unsigned long BEACON_INTERVAL_MS = 5000;
uint32_t beaconSeq = 0;
uint32_t loraTxCount = 0;
int lastTxCode = 0;

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
    .ok {
      color: #8cffb3;
      font-weight: 700;
    }
    .bad {
      color: #ff8c8c;
      font-weight: 700;
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
      <p>LoRa: <span id="lora">...</span></p>
      <p>Частота: <span id="freq">...</span> MHz</p>
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
            <td colspan="5" class="muted">Маяки соседей пока не включены</td>
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
    document.getElementById('freq').textContent = s.lora_freq_mhz;

    const lora = document.getElementById('lora');
    if (s.lora_ok) {
      lora.textContent = 'OK';
      lora.className = 'ok';
    } else {
      lora.textContent = 'ERROR ' + s.lora_begin_code;
      lora.className = 'bad';
    }
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
  StaticJsonDocument<768> doc;

  doc["callsign"] = CALLSIGN;
  doc["ip"] = WiFi.softAPIP().toString();
  doc["uptime_ms"] = millis();
  doc["wifi_mode"] = "AP";
  doc["lora_ok"] = loraOk;
  doc["lora_begin_code"] = loraBeginCode;
  doc["lora_freq_mhz"] = loraFreqMhz;
  doc["beacon_seq"] = beaconSeq;
  doc["lora_tx_count"] = loraTxCount;
  doc["last_tx_code"] = lastTxCode;

  String out;
  serializeJson(doc, out);
  server.send(200, "application/json; charset=utf-8", out);
}

void handlePeers() {
  server.send(200, "application/json; charset=utf-8", "[]");
}

void setupLoRa() {
  Serial.println("Starting LoRa...");

  pinMode(LORA_ANT_SW, OUTPUT);
  digitalWrite(LORA_ANT_SW, HIGH);

  loraSPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);

  loraBeginCode = radio.begin(loraFreqMhz);

  Serial.print("radio.begin -> ");
  Serial.println(loraBeginCode);

  if (loraBeginCode == RADIOLIB_ERR_NONE) {
    loraOk = true;
    Serial.println("LoRa OK");
    radio.startReceive();
  } else {
    loraOk = false;
    Serial.println("LoRa ERROR");
  }
}

void sendBeacon() {
  if (!loraOk) {
    return;
  }

  beaconSeq++;

  String msg = "BEACON|";
  msg += CALLSIGN;
  msg += "|";
  msg += WiFi.softAPIP().toString();
  msg += "|";
  msg += String(beaconSeq);
  msg += "|";
  msg += String(millis());

  Serial.print("TX beacon: ");
  Serial.println(msg);

  lastTxCode = radio.transmit(msg);

  Serial.print("transmit -> ");
  Serial.println(lastTxCode);

  if (lastTxCode == RADIOLIB_ERR_NONE) {
    loraTxCount++;
  }

  radio.startReceive();
}

void setupWeb() {
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

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("=== LoRa Link Test Web ===");

  setupLoRa();
  setupWeb();
}

void loop() {
  server.handleClient();

  unsigned long now = millis();
  if (now - lastBeaconMs >= BEACON_INTERVAL_MS) {
    lastBeaconMs = now;
    sendBeacon();
  }
}
