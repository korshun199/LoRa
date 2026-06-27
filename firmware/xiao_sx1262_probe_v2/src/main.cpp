#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>

#define LORA_SCK     7
#define LORA_MISO    8
#define LORA_MOSI    9
#define LORA_NSS     41
#define LORA_DIO1    39
#define LORA_NRST    42
#define LORA_BUSY    40
#define LORA_ANT_SW  38

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

int tryBegin(float tcxo) {
  Serial.println();
  Serial.print("[SX1262] try radio.begin TCXO=");
  Serial.println(tcxo);

  int state = radio.begin(
    868.0,
    125.0,
    9,
    7,
    0x12,
    10,
    8,
    tcxo
  );

  printResult("[SX1262] radio.begin", state);
  return state;
}

void setup() {
  Serial.begin(115200);
  delay(3000);

  Serial.println();
  Serial.println("=== LoRa Project: XIAO ESP32S3 + Wio SX1262 Probe V2 ===");

  Serial.printf("[SPI] SCK=%d MISO=%d MOSI=%d NSS=%d\n", LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);
  Serial.printf("[RADIO] DIO1=%d NRST=%d BUSY=%d ANT_SW=%d\n", LORA_DIO1, LORA_NRST, LORA_BUSY, LORA_ANT_SW);

  pinMode(LORA_ANT_SW, OUTPUT);
  digitalWrite(LORA_ANT_SW, HIGH);
  Serial.println("[ANT_SW] HIGH");

  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);
  Serial.println("[SPI] begin custom pins");

  int state = tryBegin(1.6);

  if (state != RADIOLIB_ERR_NONE) {
    state = tryBegin(1.8);
  }

  if (state != RADIOLIB_ERR_NONE) {
    state = tryBegin(0.0);
  }

  if (state == RADIOLIB_ERR_NONE) {
    Serial.println();
    Serial.println("[SUCCESS] SX1262 detected and initialized");
    printResult("[SX1262] standby", radio.standby());
  } else {
    Serial.println();
    Serial.println("[ERROR] SX1262 still not initialized");
    Serial.println("[CHECK] soldering, GPIO mapping, 3V3, GND, NSS, SCK, MOSI, MISO, BUSY, DIO1, NRST");
  }
}

void loop() {
  Serial.println("[ALIVE] probe v2 running");
  delay(5000);
}
