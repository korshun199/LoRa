#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>

#define LORA_DIO1    D0
#define LORA_BUSY    D1
#define LORA_NRST    D2
#define LORA_NSS     D3
#define LORA_ANT_SW  D4
#define LORA_SCK     D8
#define LORA_MISO    D9
#define LORA_MOSI    D10

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
  Serial.println("=== LoRa Project: XIAO SX1262 PING RX ===");

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

  Serial.println("[READY] LoRa RX ready");
}

void loop() {
  String payload;

  int state = radio.receive(payload);

  if (state == RADIOLIB_ERR_NONE) {
    Serial.println();
    Serial.println("[RX] packet received");
    Serial.print("[RX] payload: ");
    Serial.println(payload);
    Serial.print("[RX] RSSI: ");
    Serial.print(radio.getRSSI());
    Serial.println(" dBm");
    Serial.print("[RX] SNR: ");
    Serial.print(radio.getSNR());
    Serial.println(" dB");
    Serial.print("[RX] Frequency error: ");
    Serial.print(radio.getFrequencyError());
    Serial.println(" Hz");
  } else if (state == RADIOLIB_ERR_RX_TIMEOUT) {
    Serial.println("[RX] timeout");
  } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
    Serial.println("[RX] CRC mismatch");
  } else {
    Serial.print("[RX] failed -> ");
    Serial.println(state);
  }

  delay(100);
}
