#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>

struct PinMap {
  const char* name;
  int nss;
  int dio1;
  int nrst;
  int busy;
};

// Known and suspected mappings for XIAO ESP32S3 + Wio SX1262 variants.
// We test them one by one because human civilization apparently needed
// multiple incompatible tiny boards with the same marketing name.
PinMap maps[] = {
  {"RadioLib discussion Wio-XIAO", 5, 2, 3, 4},
  {"Alt issue example 10-2-3-9", 10, 2, 3, 9},
  {"Alt raw Module 8-14-12-13", 8, 14, 12, 13},
  {"Common XIAO D mapping 44-2-3-4", 44, 2, 3, 4},
  {"Common XIAO D mapping 41-2-3-4", 41, 2, 3, 4},
  {"Swap DIO/BUSY 5-4-3-2", 5, 4, 3, 2},
  {"Swap reset/busy 5-2-4-3", 5, 2, 4, 3},
};

void printResult(int state) {
  Serial.print("result=");
  Serial.print(state);

  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("  [OK]");
  } else {
    Serial.println("  [FAIL]");
  }
}

void setup() {
  Serial.begin(115200);
  delay(3000);

  Serial.println();
  Serial.println("=== LoRa Project: SX1262 Pin Scan ===");
  Serial.println("[INFO] If one mapping returns 0, SX1262 is detected.");
  Serial.println("[SPI] begin");
  SPI.begin();

  for (size_t i = 0; i < sizeof(maps) / sizeof(maps[0]); i++) {
    PinMap m = maps[i];

    Serial.println();
    Serial.println("----------------------------------------");
    Serial.print("[TEST] ");
    Serial.println(m.name);
    Serial.printf("[PINS] NSS=%d DIO1=%d NRST=%d BUSY=%d\n", m.nss, m.dio1, m.nrst, m.busy);

    SX1262 radio = new Module(m.nss, m.dio1, m.nrst, m.busy);

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

    printResult(state);

    if (state == RADIOLIB_ERR_NONE) {
      Serial.println("[FOUND] SX1262 detected with this mapping.");
      radio.standby();
      break;
    }

    delay(1000);
  }

  Serial.println();
  Serial.println("=== SCAN DONE ===");
}

void loop() {
  delay(5000);
  Serial.println("[ALIVE] pin scan firmware running");
}
