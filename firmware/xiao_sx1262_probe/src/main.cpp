#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>

// Wio SX1262 for XIAO ESP32S3 known working mapping:
// NSS=5, DIO1=2, NRST=3, BUSY=4
#define LORA_NSS   5
#define LORA_DIO1  2
#define LORA_NRST  3
#define LORA_BUSY  4

SX1262 radio = new Module(LORA_NSS, LORA_DIO1, LORA_NRST, LORA_BUSY);

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
  Serial.println("=== LoRa Project: XIAO ESP32S3 + Wio SX1262 Probe ===");
  Serial.printf("[PIN] NSS=%d DIO1=%d NRST=%d BUSY=%d\n", LORA_NSS, LORA_DIO1, LORA_NRST, LORA_BUSY);

  Serial.println("[SPI] begin");
  SPI.begin();

  Serial.println("[SX1262] begin");

  int state = radio.begin(
    868.0,    // frequency MHz, diagnostic only
    125.0,    // bandwidth kHz
    9,        // spreading factor
    7,        // coding rate
    0x12,     // sync word
    10,       // output power dBm
    8,        // preamble length
    1.6       // TCXO voltage, harmless if unsupported by board
  );

  printResult("[SX1262] radio.begin", state);

  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("[SX1262] detected and initialized");
    Serial.println("[SX1262] setting standby");
    printResult("[SX1262] standby", radio.standby());

    Serial.println("[SX1262] diagnostic complete");
  } else {
    Serial.println("[SX1262] not initialized");
    Serial.println("[HINT] Check board orientation, antenna, and pin mapping.");
  }
}

void loop() {
  Serial.println("[ALIVE] SX1262 probe running");
  delay(5000);
}
