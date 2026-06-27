#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"

String deviceId;
unsigned long lastStatusMs = 0;
const unsigned long STATUS_INTERVAL_MS = 10000;

String makeDeviceId() {
  uint64_t mac = ESP.getEfuseMac();
  char buf[64];

  snprintf(
    buf,
    sizeof(buf),
    "%s-%04X%08X",
    DEVICE_PREFIX,
    (uint16_t)(mac >> 32),
    (uint32_t)mac
  );

  return String(buf);
}

String httpPost(const String& url, const String& payload) {
  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  int code = http.POST(payload);
  String body = http.getString();

  Serial.printf("[HTTP POST] %s -> %d\n", url.c_str(), code);
  Serial.println(body);

  http.end();
  return body;
}

String httpGet(const String& url) {
  HTTPClient http;
  http.begin(url);

  int code = http.GET();
  String body = http.getString();

  Serial.printf("[HTTP GET] %s -> %d\n", url.c_str(), code);
  Serial.println(body);

  http.end();
  return body;
}

void connectWiFi() {
  Serial.printf("[WiFi] connecting to: %s\n", WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int tries = 0;

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    tries++;

    if (tries > 60) {
      Serial.println();
      Serial.println("[WiFi] connection failed, restarting");
      ESP.restart();
    }
  }

  Serial.println();
  Serial.print("[WiFi] connected, IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("[WiFi] RSSI: ");
  Serial.println(WiFi.RSSI());
}

void registerDevice() {
  String url = String(SERVER_BASE_URL) + "/api/devices/register";

  String payload =
    String("{") +
    "\"device_id\":\"" + deviceId + "\"," +
    "\"device_type\":\"xiao_esp32s3_wio_sx1262\"," +
    "\"firmware_version\":\"wifi-heartbeat-0.1.0\"" +
    "}";

  httpPost(url, payload);
}

void sendStatus() {
  String url = String(SERVER_BASE_URL) + "/api/devices/" + deviceId + "/status";

  String payload =
    String("{") +
    "\"device_id\":\"" + deviceId + "\"," +
    "\"status\":\"online\"," +
    "\"message\":\"xiao wifi heartbeat\"," +
    "\"battery\":100," +
    "\"rssi\":" + String(WiFi.RSSI()) +
    "}";

  httpPost(url, payload);
}

String extractMsgId(const String& body) {
  String key = "\"msg_id\":\"";
  int pos = body.indexOf(key);

  if (pos < 0) {
    return "";
  }

  int start = pos + key.length();
  int end = body.indexOf("\"", start);

  if (end < 0) {
    return "";
  }

  return body.substring(start, end);
}

void checkCommandAndAck() {
  String url = String(SERVER_BASE_URL) + "/api/devices/" + deviceId + "/command";
  String body = httpGet(url);

  String msgId = extractMsgId(body);

  if (msgId.length() == 0) {
    Serial.println("[CMD] no command");
    return;
  }

  Serial.print("[CMD] received msg_id: ");
  Serial.println(msgId);

  String ackUrl = String(SERVER_BASE_URL) + "/api/devices/" + deviceId + "/command/ack";

  String payload =
    String("{") +
    "\"device_id\":\"" + deviceId + "\"," +
    "\"msg_id\":\"" + msgId + "\"," +
    "\"result\":\"done\"," +
    "\"message\":\"xiao firmware acknowledged command\"" +
    "}";

  httpPost(ackUrl, payload);
}

void setup() {
  Serial.begin(115200);
  delay(3000);

  Serial.println();
  Serial.println("=== LoRa Project: XIAO ESP32S3 WiFi Gateway Test ===");

  deviceId = makeDeviceId();

  Serial.print("[DEVICE] ");
  Serial.println(deviceId);

  connectWiFi();
  registerDevice();
  sendStatus();
  checkCommandAndAck();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WiFi] disconnected, reconnecting");
    connectWiFi();
  }

  unsigned long now = millis();

  if (now - lastStatusMs >= STATUS_INTERVAL_MS) {
    lastStatusMs = now;

    sendStatus();
    checkCommandAndAck();
  }

  delay(100);
}
