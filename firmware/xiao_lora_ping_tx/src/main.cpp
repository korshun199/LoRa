#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>

// Confirmed real soldered mapping, 2026-06-27:
// LoRa MOSI -> XIAO D10
// LoRa MISO -> XIAO D9
// LoRa SCK  -> XIAO D8
// LoRa SW   -> XIAO D4
// LoRa NSS  -> XIAO D3
// LoRa RST  -> XIAO D2
// LoRa BUSY -> XIAO D1
// LoRa DIO1 -> XIAO D0

#define LORA_DIO1    D0
#define LORA_BUSY    D1
#define LORA_NRST    D2
#define LORA_NSS     D3
#define LORA_ANT_SW  D4
#define LORA_SCK     D8
#define LORA_MISO    D9
#define LORA_MOSI    D10

SX1262 radio = new Module(LORA_NSS, LORA_DIO1, LORA_NRST, LORA_BUSY);

uint32_t counter = 0;
unsigned long lastTxMs = 0;
const unsigned long TX_INTERVAL_MS = 5000;

void printResult(const char* label, int state) {
  Serial.print(label);
  Serial.print(" -> ");
  Serial.println(state);

  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("[OK]");
  } else {
    Serial.println("[FAIL]");
  }
}

void setup() {
  Serial.begin(115200);
  delay(3000);

  Serial.println();
  Serial.println("=== LoRa Project: XIAO SX1262 PING TX ===");

  Serial.printf("[PINS] DIO1=D0(%d), BUSY=D1(%d), NRST=D2(%d), NSS=D3(%d), ANT_SW=D4(%d)\n",
                LORA_DIO1, LORA_BUSY, LORA_NRST, LORA_NSS, LORA_ANT_SW);

  Serial.printf("[SPI] SCK=D8(%d), MISO=D9(%d), MOSI=D10(%d)\n",
                LORA_SCK, LORA_MISO, LORA_MOSI);

  pinMode(LORA_ANT_SW, OUTPUT);
  digitalWrite(LORA_ANT_SW, HIGH);
  Serial.println("[ANT_SW] HIGH");

  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);
  Serial.println("[SPI] begin");

  int state = radio.begin(
    868.0,
    125.0,
    9,
    7,
    0x12,
    10,
    8,
    1.6
  );

  printResult("[SX1262] radio.begin", state);

  if (state != RADIOLIB_ERR_NONE) {
    Serial.println("[ERROR] radio init failed, stop");
    while (true) {
      delay(5000);
      Serial.println("[DEAD] radio init failed");
    }
  }

  printResult("[SX1262] standby", radio.standby());

  Serial.println("[READY] LoRa TX beacon ready");
}

void loop() {
  unsigned long now = millis();

  if (now - lastTxMs >= TX_INTERVAL_MS) {
    lastTxMs = now;
    counter++;

    String payload = "PING #" + String(counter) + " from xiao-wio";

    Serial.print("[TX] ");
    Serial.println(payload);

    int state = radio.transmit(payload);

    printResult("[TX] transmit", state);
  }

  delay(50);
}
