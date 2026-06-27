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

uint32_t counter = 0;
unsigned long lastAskMs = 0;

void radioInit() {
  pinMode(LORA_ANT_SW, OUTPUT);
  digitalWrite(LORA_ANT_SW, HIGH);

  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);

  int state = radio.begin(868.0, 125.0, 9, 7, 0x12, 10, 8, 1.6);

  Serial.print("[RADIO] start -> ");
  Serial.println(state);

  if (state != RADIOLIB_ERR_NONE) {
    Serial.println("💀 Радио не поднялось. Паяльник опять где-то смеётся.");
    while (true) {
      delay(3000);
      Serial.println("💀 Всё ещё мёртвое радио.");
    }
  }

  radio.standby();
}

void setup() {
  Serial.begin(115200);
  delay(3000);

  Serial.println();
  Serial.println("======================================");

#ifdef NODE_BRATAN
  Serial.println("😎 БРАТАН проснулся.");
  Serial.println("Рация включена. Ищу ЧУВАКА...");
#else
  Serial.println("🤠 ЧУВАК проснулся.");
  Serial.println("Антенна на месте. Жду БРАТАНА...");
#endif

  Serial.println("======================================");

  radioInit();

#ifdef NODE_BRATAN
  Serial.println("😎 БРАТАН: Работаем. Сейчас проверим, жив ли этот болтун.");
#else
  Serial.println("🤠 ЧУВАК: Я на приёме. Главное, чтобы меня опять не перепрошили.");
#endif
}

void loop() {
#ifdef NODE_BRATAN
  unsigned long now = millis();

  if (now - lastAskMs >= 5000) {
    lastAskMs = now;
    counter++;

    String question = "BRATAN>CHUVAK #" + String(counter) + ": Чувак, ты где? Приём!";

    Serial.println();
    Serial.println("😎 БРАТАН говорит:");
    Serial.println(question);

    int tx = radio.transmit(question);

    if (tx == RADIOLIB_ERR_NONE) {
      Serial.println("📡 БРАТАН: Вопрос улетел. Жду ответ, как зарплату.");

      String answer;
      int rx = radio.receive(answer);

      if (rx == RADIOLIB_ERR_NONE) {
        Serial.println("✅ БРАТАН получил ответ:");
        Serial.println(answer);
        Serial.print("📶 RSSI: ");
        Serial.print(radio.getRSSI());
        Serial.print(" dBm, SNR: ");
        Serial.print(radio.getSNR());
        Serial.println(" dB");
        Serial.println("😎 БРАТАН: Нормально. Чувак живой.");
      } else if (rx == RADIOLIB_ERR_RX_TIMEOUT) {
        Serial.println("⏳ БРАТАН: Тишина. Чувак опять ушёл за хлебом.");
      } else {
        Serial.print("⚠️ БРАТАН: Ошибка приёма: ");
        Serial.println(rx);
      }
    } else {
      Serial.print("⚠️ БРАТАН: Не смог отправить: ");
      Serial.println(tx);
    }
  }

  delay(100);

#else
  String incoming;
  int rx = radio.receive(incoming);

  if (rx == RADIOLIB_ERR_NONE) {
    Serial.println();
    Serial.println("🤠 ЧУВАК услышал:");
    Serial.println(incoming);

    Serial.print("📶 RSSI: ");
    Serial.print(radio.getRSSI());
    Serial.print(" dBm, SNR: ");
    Serial.print(radio.getSNR());
    Serial.println(" dB");

    if (incoming.indexOf("BRATAN>CHUVAK") >= 0) {
      counter++;
      String answer = "CHUVAK>BRATAN #" + String(counter) + ": Братан, слышу тебя отлично. Эфир чистый, настроение боевое.";

      Serial.println("🤠 ЧУВАК отвечает:");
      Serial.println(answer);

      int tx = radio.transmit(answer);

      if (tx == RADIOLIB_ERR_NONE) {
        Serial.println("✅ ЧУВАК: Ответ отправил. Не зря паяли.");
      } else {
        Serial.print("⚠️ ЧУВАК: Ответ не ушёл: ");
        Serial.println(tx);
      }
    }
  } else if (rx == RADIOLIB_ERR_RX_TIMEOUT) {
    Serial.println("🤠 ЧУВАК: Слушаю эфир. Пока тишина, только электроны шуршат.");
  } else {
    Serial.print("⚠️ ЧУВАК: Ошибка приёма: ");
    Serial.println(rx);
  }

  delay(100);
#endif
}
