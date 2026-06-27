#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>

// Real soldered mapping:
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
  Serial.println("=== LoRa Project: SX1262 Probe V3 Real Pins ===");

  Serial.printf("[PINS] DIO1=D0(%d), BUSY=D1(%d), NRST=D2(%d), NSS=D3(%d), ANT_SW=D4(%d)\n",
                LORA_DIO1, LORA_BUSY, LORA_NRST, LORA_NSS, LORA_ANT_SW);

  Serial.printf("[SPI] SCK=D8(%d), MISO=D9(%d), MOSI=D10(%d)\n",
                LORA_SCK, LORA_MISO, LORA_MOSI);

  pinMode(LORA_ANT_SW, OUTPUT);
  digitalWrite(LORA_ANT_SW, HIGH);
  Serial.println("[ANT_SW] HIGH");

  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);
  Serial.println("[SPI] begin real soldered pins");

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
    Serial.println("[ERROR] SX1262 still not initialized on real soldered pins");
    Serial.println("[NEXT] If soldering is OK, test raw SPI and XIAO Dx-to-GPIO mapping.");
  }
}

void loop() {
  Serial.println("[ALIVE] probe v3 real pins running");
  delay(5000);
}
