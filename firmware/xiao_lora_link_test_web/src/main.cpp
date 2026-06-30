#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <RadioLib.h>
#include <Preferences.h>

// ===== Device config =====
Preferences prefs;

String callsign = "BRATAN";
String apSsid = "LORA-BRATAN";
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
uint32_t loraRxCount = 0;
int lastTxCode = 0;
int lastRxCode = 0;

struct PeerInfo {
  bool active;
  String callsign;
  String ip;
  uint32_t firstSeq;
  uint32_t lastSeq;
  uint32_t rxCount;
  uint32_t lostCount;
  uint8_t quality;
  float rssi;
  float snr;
  uint32_t lastSeenMs;
};

const int MAX_PEERS = 16;
PeerInfo peers[MAX_PEERS];

volatile bool loraPacketReceived = false;

#if defined(ESP32)
void IRAM_ATTR setLoraPacketReceivedFlag() {
  loraPacketReceived = true;
}
#else
void setLoraPacketReceivedFlag() {
  loraPacketReceived = true;
}
#endif

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

async function updatePeers() {
  try {
    const r = await fetch('/api/peers');
    const peers = await r.json();
    const tbody = document.getElementById('peers');

    if (!peers.length) {
      tbody.innerHTML = '<tr><td colspan="5" class="muted">Соседи пока не слышны</td></tr>';
      return;
    }

    tbody.innerHTML = peers.map(p => `
      <tr>
        <td>${p.callsign}</td>
        <td>${p.ip}</td>
        <td>${p.rssi}</td>
        <td>${p.snr}</td>
        <td>${p.quality}% / ${Math.floor(p.last_seen_ms / 1000)} сек</td>
      </tr>
    `).join('');
  } catch(e) {
    console.log(e);
  }
}

setInterval(updateStatus, 1000);
setInterval(updatePeers, 1000);
updateStatus();
updatePeers();
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

  doc["callsign"] = callsign;
  doc["ip"] = WiFi.softAPIP().toString();
  doc["uptime_ms"] = millis();
  doc["wifi_mode"] = "AP";
  doc["lora_ok"] = loraOk;
  doc["lora_begin_code"] = loraBeginCode;
  doc["lora_freq_mhz"] = loraFreqMhz;
  doc["beacon_seq"] = beaconSeq;
  doc["lora_tx_count"] = loraTxCount;
  doc["lora_rx_count"] = loraRxCount;
  doc["last_tx_code"] = lastTxCode;
  doc["last_rx_code"] = lastRxCode;

  String out;
  serializeJson(doc, out);
  server.send(200, "application/json; charset=utf-8", out);
}

void handlePeers() {
  StaticJsonDocument<2048> doc;
  JsonArray arr = doc.to<JsonArray>();

  uint32_t now = millis();

  for (int i = 0; i < MAX_PEERS; i++) {
    if (!peers[i].active) {
      continue;
    }

    JsonObject item = arr.add<JsonObject>();
    item["callsign"] = peers[i].callsign;
    item["ip"] = peers[i].ip;
    item["first_seq"] = peers[i].firstSeq;
    item["last_seq"] = peers[i].lastSeq;
    item["rx_count"] = peers[i].rxCount;
    item["lost_count"] = peers[i].lostCount;
    item["rssi"] = peers[i].rssi;
    item["snr"] = peers[i].snr;
    item["last_seen_ms"] = now - peers[i].lastSeenMs;
    item["quality"] = peers[i].quality;
  }

  String out;
  serializeJson(doc, out);
  server.send(200, "application/json; charset=utf-8", out);
}

int findPeerSlot(const String& peerCallsign) {
  int freeSlot = -1;

  for (int i = 0; i < MAX_PEERS; i++) {
    if (peers[i].active && peers[i].callsign == peerCallsign) {
      return i;
    }

    if (!peers[i].active && freeSlot < 0) {
      freeSlot = i;
    }
  }

  return freeSlot;
}

void updatePeerFromBeacon(const String& peerCallsign, const String& peerIp, uint32_t seq, float rssi, float snr) {
  if (peerCallsign == callsign) {
    return;
  }

  int slot = findPeerSlot(peerCallsign);
  if (slot < 0) {
    Serial.println("PEER_TABLE_FULL");
    return;
  }

  if (!peers[slot].active) {
    peers[slot].active = true;
    peers[slot].callsign = peerCallsign;
    peers[slot].ip = peerIp;
    peers[slot].firstSeq = seq;
    peers[slot].lastSeq = seq;
    peers[slot].rxCount = 0;
    peers[slot].lostCount = 0;
    peers[slot].quality = 100;
  }

  peers[slot].ip = peerIp;
  peers[slot].lastSeq = seq;
  peers[slot].rxCount++;

  uint32_t expected = 1;
  if (peers[slot].lastSeq >= peers[slot].firstSeq) {
    expected = peers[slot].lastSeq - peers[slot].firstSeq + 1;
  }

  if (expected > peers[slot].rxCount) {
    peers[slot].lostCount = expected - peers[slot].rxCount;
  } else {
    peers[slot].lostCount = 0;
  }

  if (expected > 0) {
    peers[slot].quality = (uint8_t)((peers[slot].rxCount * 100UL) / expected);
  } else {
    peers[slot].quality = 0;
  }

  peers[slot].rssi = rssi;
  peers[slot].snr = snr;
  peers[slot].lastSeenMs = millis();

  Serial.print("PEER ");
  Serial.print(peerCallsign);
  Serial.print(" ");
  Serial.print(peerIp);
  Serial.print(" seq=");
  Serial.print(seq);
  Serial.print(" rssi=");
  Serial.print(rssi);
  Serial.print(" snr=");
  Serial.print(snr);
  Serial.print(" quality=");
  Serial.print(peers[slot].quality);
  Serial.print("% lost=");
  Serial.println(peers[slot].lostCount);
}

void parseBeacon(const String& msg, float rssi, float snr) {
  if (!msg.startsWith("BEACON|")) {
    return;
  }

  int p1 = msg.indexOf('|');
  int p2 = msg.indexOf('|', p1 + 1);
  int p3 = msg.indexOf('|', p2 + 1);
  int p4 = msg.indexOf('|', p3 + 1);

  if (p1 < 0 || p2 < 0 || p3 < 0 || p4 < 0) {
    Serial.println("BAD_BEACON_FORMAT");
    return;
  }

  String peerCallsign = msg.substring(p1 + 1, p2);
  String peerIp = msg.substring(p2 + 1, p3);
  uint32_t seq = msg.substring(p3 + 1, p4).toInt();

  updatePeerFromBeacon(peerCallsign, peerIp, seq, rssi, snr);
}

void pollLoRaReceive() {
  if (!loraOk) {
    return;
  }

  if (!loraPacketReceived) {
    return;
  }

  loraPacketReceived = false;

  String rx;
  lastRxCode = radio.readData(rx);

  if (lastRxCode == RADIOLIB_ERR_NONE) {
    loraRxCount++;

    float rssi = radio.getRSSI();
    float snr = radio.getSNR();

    Serial.print("RX: ");
    Serial.println(rx);

    parseBeacon(rx, rssi, snr);
  } else {
    Serial.print("RX readData error -> ");
    Serial.println(lastRxCode);
  }

  radio.startReceive();
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
    radio.setDio1Action(setLoraPacketReceivedFlag);
    radio.startReceive();
  } else {
    loraOk = false;
    Serial.println("LoRa ERROR");
  }
}

bool parseIpString(const String& ipText, IPAddress& outIp) {
  int parts[4] = {0, 0, 0, 0};
  int partIndex = 0;
  String current = "";

  for (size_t i = 0; i < ipText.length(); i++) {
    char c = ipText[i];

    if (c == '.') {
      if (partIndex >= 3 || current.length() == 0) {
        return false;
      }
      parts[partIndex++] = current.toInt();
      current = "";
    } else if (isDigit(c)) {
      current += c;
    } else {
      return false;
    }
  }

  if (partIndex != 3 || current.length() == 0) {
    return false;
  }

  parts[partIndex] = current.toInt();

  for (int i = 0; i < 4; i++) {
    if (parts[i] < 0 || parts[i] > 255) {
      return false;
    }
  }

  outIp = IPAddress(parts[0], parts[1], parts[2], parts[3]);
  return true;
}

void rebuildApSsid() {
  apSsid = "LORA-" + callsign;
}

void loadConfig() {
  prefs.begin("lora-link", true);

  callsign = prefs.getString("callsign", "BRATAN");
  String ipText = prefs.getString("ip", "192.168.4.10");

  prefs.end();

  if (!parseIpString(ipText, local_ip)) {
    local_ip = IPAddress(192, 168, 4, 10);
  }

  gateway = local_ip;
  rebuildApSsid();

  Serial.println("Loaded config:");
  Serial.print("CALLSIGN=");
  Serial.println(callsign);
  Serial.print("IP=");
  Serial.println(local_ip);
  Serial.print("SSID=");
  Serial.println(apSsid);
}

void saveConfig(const String& newCallsign, const String& newIpText) {
  IPAddress parsedIp;

  if (newCallsign.length() < 1 || newCallsign.length() > 24) {
    Serial.println("CONFIG_ERROR BAD_CALLSIGN");
    return;
  }

  if (!parseIpString(newIpText, parsedIp)) {
    Serial.println("CONFIG_ERROR BAD_IP");
    return;
  }

  prefs.begin("lora-link", false);
  prefs.putString("callsign", newCallsign);
  prefs.putString("ip", newIpText);
  prefs.end();

  Serial.print("CONFIG_OK ");
  Serial.print(newCallsign);
  Serial.print(" ");
  Serial.println(newIpText);

  Serial.println("REBOOTING");
  delay(500);
  ESP.restart();
}

void handleSerialCommand() {
  if (!Serial.available()) {
    return;
  }

  String line = Serial.readStringUntil('\n');
  line.trim();

  if (line.length() == 0) {
    return;
  }

  Serial.print("SERIAL_CMD ");
  Serial.println(line);

  if (line == "SHOW") {
    Serial.print("CALLSIGN=");
    Serial.println(callsign);
    Serial.print("IP=");
    Serial.println(local_ip);
    Serial.print("SSID=");
    Serial.println(apSsid);
    return;
  }

  if (line.startsWith("CONFIG ")) {
    int firstSpace = line.indexOf(' ');
    int secondSpace = line.indexOf(' ', firstSpace + 1);

    if (secondSpace < 0) {
      Serial.println("CONFIG_ERROR USAGE CONFIG CALLSIGN IP");
      return;
    }

    String newCallsign = line.substring(firstSpace + 1, secondSpace);
    String newIpText = line.substring(secondSpace + 1);
    newCallsign.trim();
    newIpText.trim();

    saveConfig(newCallsign, newIpText);
    return;
  }

  Serial.println("UNKNOWN_CMD");
  Serial.println("AVAILABLE: SHOW | CONFIG CALLSIGN IP");
}

void sendBeacon() {
  if (!loraOk) {
    return;
  }

  beaconSeq++;

  String msg = "BEACON|";
  msg += callsign;
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
  WiFi.softAP(apSsid.c_str(), AP_PASS);

  Serial.print("AP SSID: ");
  Serial.println(apSsid);
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

  loadConfig();
  setupLoRa();
  setupWeb();
}

void loop() {
  server.handleClient();
  handleSerialCommand();
  pollLoRaReceive();

  unsigned long now = millis();
  if (now - lastBeaconMs >= BEACON_INTERVAL_MS) {
    lastBeaconMs = now;
    sendBeacon();
  }
}
